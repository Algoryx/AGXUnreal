#include "Contacts/ContactEventListener.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AgxDynamicsObjectsAccess.h"
#include "AGX_LogCategory.h"
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/Contacts/ShapeContactEntity.h"
#include "Shapes/AnyShapeBarrier.h"
#include "SimulationBarrier.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
// Note the BeginAGXIncludes.h and EndAGXIncludes.h wrapping the AGX Dynamics header files.
#include "BeginAGXIncludes.h"
#include "agxCollide/Contacts.h"
#include <agxSDK/Simulation.h>
#include "EndAGXIncludes.h"

ContactEventListener::ContactEventListener(
	FSimulationBarrier& Simulation,
	TFunction<EAGX_KeepContactPolicy(double, FShapeContactBarrier&)> InImpactCallback,
	TFunction<EAGX_KeepContactPolicy(double, FShapeContactBarrier&)> InContactCallback,
	TFunction<void(double, FAnyShapeBarrier&, FAnyShapeBarrier&)> InSeparationCallback)
	: agxSDK::ContactEventListener(ActivationMask::ALL)
	, ImpactCallback(InImpactCallback)
	, ContactCallback(InContactCallback)
	, SeparationCallback(InSeparationCallback)
{
	if (!Simulation.HasNative())
	{
		UE_LOG(
			LogAGX, Warning, TEXT("ContactEventListener got Simulation Barrier without Native."));
		return;
	}

	agxSDK::Simulation* SimulationAGX = FAGX_AgxDynamicsObjectsAccess::GetFrom(&Simulation);
	if (SimulationAGX == nullptr)
	{
		UE_LOG(LogAGX, Warning, TEXT("ContactEventListener got nullptr AGX Dynamics Simulation."));
		return;
	}

	SimulationAGX->add(this);
}

//~ Begin agxSDK::ContactEventListener interface.

agxSDK::ContactEventListener::KeepContactPolicy ContactEventListener::impact(
	const agx::TimeStamp& TimeStamp, agxCollide::GeometryContact* GeometryContact)
{
	if (GeometryContact == nullptr)
		return KEEP_CONTACT;

	if (!ImpactCallback)
	{
		return KEEP_CONTACT;
	}

	// Construct a Barrier object that can be used to access the AGX Dynamics Geometry Contact
	// from non-Barrier modules and pass it to the registered callback.
	FShapeContactBarrier ShapeContact(std::make_unique<FShapeContactEntity>(*GeometryContact));
	return Convert(ImpactCallback(TimeStamp, ShapeContact));
}

agxSDK::ContactEventListener::KeepContactPolicy ContactEventListener::contact(
	const agx::TimeStamp& TimeStamp, agxCollide::GeometryContact* GeometryContact)
{
	if (GeometryContact == nullptr)
		return KEEP_CONTACT;

	if (!ContactCallback)
	{
		return KEEP_CONTACT;
	}

	// Construct a Barrier object that can be used to access the AGX Dynamics Geometry Contact
	// from non-Barrier modules and pass it to the registered callback.
	FShapeContactBarrier ShapeContact(std::make_unique<FShapeContactEntity>(*GeometryContact));
	return Convert(ContactCallback(TimeStamp, ShapeContact));
}

void ContactEventListener::separation(
	const agx::TimeStamp& TimeStamp, agxCollide::GeometryPair& GeometryPair)
{
	if (!SeparationCallback)
	{
		return;
	}

	// Construct a pair of Barrier objects that can be used to access the AGX Dynamics Geometries
	// from non-Barrier modules and pass them to the registered callback.
	agxCollide::Geometry* First = GeometryPair.first;
	agxCollide::Geometry* Second = GeometryPair.second;
	if (First == nullptr || Second == nullptr)
		return;

	FAnyShapeBarrier FirstShape(std::make_unique<FGeometryAndShapeRef>(First, First->getShape()));
	FAnyShapeBarrier SecondShape(
		std::make_unique<FGeometryAndShapeRef>(Second, Second->getShape()));
	SeparationCallback(TimeStamp, FirstShape, SecondShape);
}

//~ End agxSDK::ContactEventListener interface.
