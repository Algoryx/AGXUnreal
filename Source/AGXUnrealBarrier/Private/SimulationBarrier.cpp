// Copyright 2024, Algoryx Simulation AB.

#include "SimulationBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGXBarrierFactories.h"
#include "AGX_LogCategory.h"
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/Wire/WireRef.h"
#include "Constraints/ConstraintBarrier.h"
#include "Materials/ContactMaterialBarrier.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "RigidBodyBarrier.h"
#include "Shapes/ShapeBarrier.h"
#include "Terrain/TerrainBarrier.h"
#include "Terrain/TerrainPagerBarrier.h"
#include "Tires/TireBarrier.h"
#include "TypeConversions.h"
#include "Wire/WireBarrier.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/PointGravityField.h>
#include <agx/Statistics.h>
#include <agx/UniformGravityField.h>
#include <agxSDK/MergeSplitHandler.h>
#include <agxSDK/Simulation.h>
#include <agxUtil/agxUtil.h>
#include "EndAGXIncludes.h"

// Unreal Engine includes.
#include "Misc/AssertionMacros.h"

FSimulationBarrier::FSimulationBarrier()
	: NativeRef {new FSimulationRef}
{
}

FSimulationBarrier::~FSimulationBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FSimulationRef.
}

bool FSimulationBarrier::Add(FConstraintBarrier& Constraint)
{
	check(HasNative());
	check(Constraint.HasNative());
	return NativeRef->Native->add(Constraint.GetNative()->Native);
}

bool FSimulationBarrier::Add(FContactMaterialBarrier& ContactMaterial)
{
	check(HasNative());
	check(ContactMaterial.HasNative());
	return NativeRef->Native->add(ContactMaterial.GetNative()->Native);
}

bool FSimulationBarrier::Add(FRigidBodyBarrier& Body)
{
	check(HasNative());
	check(Body.HasNative());
	return NativeRef->Native->add(Body.GetNative()->Native, /*addGeometries*/ false);
}

bool FSimulationBarrier::Add(FShapeBarrier& Shape)
{
	check(HasNative());
	check(Shape.HasNative());
	return NativeRef->Native->add(Shape.GetNative()->NativeGeometry);
}

bool FSimulationBarrier::Add(FShapeMaterialBarrier& Material)
{
	check(HasNative());
	check(Material.HasNative());
	return NativeRef->Native->add(Material.GetNative()->Native);
}

bool FSimulationBarrier::Add(FTerrainBarrier& Terrain)
{
	check(HasNative());
	check(Terrain.HasNative());
	return NativeRef->Native->add(Terrain.GetNative()->Native);
}

bool FSimulationBarrier::Add(FTerrainPagerBarrier& TerrainPager)
{
	check(HasNative());
	check(TerrainPager.HasNative());
	return NativeRef->Native->add(TerrainPager.GetNative()->Native);
}

bool FSimulationBarrier::Add(FTireBarrier& Tire)
{
	check(HasNative());
	check(Tire.HasNative());
	return NativeRef->Native->add(Tire.GetNative()->Native);
}

bool FSimulationBarrier::Add(FWireBarrier& Wire)
{
	check(HasNative());
	check(Wire.HasNative());
	return NativeRef->Native->add(Wire.GetNative()->Native);
}

bool FSimulationBarrier::Remove(FConstraintBarrier& Constraint)
{
	check(HasNative());
	check(Constraint.HasNative());
	return NativeRef->Native->remove(Constraint.GetNative()->Native);
}

bool FSimulationBarrier::Remove(FContactMaterialBarrier& ContactMaterial)
{
	check(HasNative());
	check(ContactMaterial.HasNative());
	return NativeRef->Native->remove(ContactMaterial.GetNative()->Native);
}

bool FSimulationBarrier::Remove(FRigidBodyBarrier& Body)
{
	check(HasNative());
	check(Body.HasNative());
	return NativeRef->Native->remove(Body.GetNative()->Native, /*removeGeometries*/ false);
}

bool FSimulationBarrier::Remove(FShapeBarrier& Shape)
{
	check(HasNative());
	check(Shape.HasNative());
	return NativeRef->Native->remove(Shape.GetNative()->NativeGeometry);
}

bool FSimulationBarrier::Remove(FShapeMaterialBarrier& Material)
{
	check(HasNative());
	check(Material.HasNative());
	return NativeRef->Native->remove(Material.GetNative()->Native);
}

bool FSimulationBarrier::Remove(FTerrainBarrier& Terrain)
{
	check(HasNative());
	check(Terrain.HasNative());
	return NativeRef->Native->remove(Terrain.GetNative()->Native);
}

bool FSimulationBarrier::Remove(FTerrainPagerBarrier& TerrainPager)
{
	check(HasNative());
	check(TerrainPager.HasNative());
	return NativeRef->Native->remove(TerrainPager.GetNative()->Native);
}

bool FSimulationBarrier::Remove(FTireBarrier& Tire)
{
	check(HasNative());
	check(Tire.HasNative());
	return NativeRef->Native->remove(Tire.GetNative()->Native);
}

bool FSimulationBarrier::Remove(FWireBarrier& Wire)
{
	check(HasNative());
	check(Wire.HasNative());
	return NativeRef->Native->remove(Wire.GetNative()->Native);
}

void FSimulationBarrier::SetEnableCollisionGroupPair(
	const FName& Group1, const FName& Group2, bool CanCollide)
{
	check(HasNative());

	// In AGXUnreal, adding a collision group pair always mean "disable collision between these
	// groups". Therefore, the collision enable flag is always set to false.
	// Note that internally, the collision group names are converted to a 32 bit unsigned int via a
	// hash function.
	NativeRef->Native->getSpace()->setEnablePair(
		StringTo32BitFnvHash(Group1.ToString()), StringTo32BitFnvHash(Group2.ToString()),
		CanCollide);
}

void FSimulationBarrier::SetEnableCollision(
	FRigidBodyBarrier& Body1, FRigidBodyBarrier& Body2, bool Enable)
{
	check(Body1.HasNative());
	check(Body2.HasNative());

	agxUtil::setEnableCollisions(Body1.GetNative()->Native, Body2.GetNative()->Native, Enable);
}

bool FSimulationBarrier::WriteAGXArchive(const FString& Filename) const
{
	check(HasNative());
	size_t NumObjectsWritten = NativeRef->Native->write(Convert(Filename));
	if (NumObjectsWritten == 0)
	{
		UE_LOG(LogAGX, Warning, TEXT("Native simulation reported zero written objects."));
		return false;
	}

	return true; /// \todo How do we determine if all objects were successfully written?
}

void FSimulationBarrier::EnableRemoteDebugging(int16 Port)
{
	check(HasNative());
	NativeRef->Native->setEnableRemoteDebugging(true, Port);
}

void FSimulationBarrier::SetEnableAMOR(bool bEnable)
{
	check(HasNative());
	NativeRef->Native->getMergeSplitHandler()->setEnable(bEnable);
}

bool FSimulationBarrier::GetEnableAMOR()
{
	check(HasNative());
	return NativeRef->Native->getMergeSplitHandler()->getEnable();
}

void FSimulationBarrier::SetTimeStep(double TimeStep)
{
	check(HasNative());
	NativeRef->Native->setTimeStep(ConvertToAGX(TimeStep));
}

double FSimulationBarrier::GetTimeStep() const
{
	check(HasNative());
	return ConvertToUnreal<double>(NativeRef->Native->getTimeStep());
}

void FSimulationBarrier::SetEnableContactWarmstarting(bool bEnable)
{
	check(HasNative());
	NativeRef->Native->getDynamicsSystem()->setEnableContactWarmstarting(bEnable);
}

bool FSimulationBarrier::GetEnableContactWarmstarting() const
{
	check(HasNative());
	return NativeRef->Native->getDynamicsSystem()->getEnableContactWarmstarting();
}

void FSimulationBarrier::SetNumPpgsIterations(int32 NumIterations)
{
	check(HasNative());
	check(NumIterations > 0);
	agx::Int NumIterationsAgx = Convert(NumIterations);
	NativeRef->Native->getSolver()->setNumPPGSRestingIterations(NumIterationsAgx);
}

int32 FSimulationBarrier::GetNumPpgsIterations() const
{
	check(HasNative());
	agx::UInt NumIterationsAgx = NativeRef->Native->getSolver()->getNumPPGSRestingIterations();
	if (NumIterationsAgx > std::numeric_limits<int32>::max())
	{
		NumIterationsAgx = std::numeric_limits<int32>::max();
		UE_LOG(
			LogAGX, Warning, TEXT("Too many PPGS resting iterations. Value clamped to %llu."),
			NumIterationsAgx);
	}
	int32 NumIterations = static_cast<int32>(NumIterationsAgx);
	return NumIterations;
}

void FSimulationBarrier::SetUniformGravity(const FVector& Gravity)
{
	check(HasNative());

	// Convert Unreal Engine's cm/s^2 to AGX Dynamics' m/s^2.
	agx::Vec3 GravityAgx = ConvertDisplacement(Gravity);
	agx::UniformGravityFieldRef Field = new agx::UniformGravityField(GravityAgx);
	NativeRef->Native->setGravityField(Field);
}

FVector FSimulationBarrier::GetUniformGravity() const
{
	agx::GravityField* GravityField = NativeRef->Native->getGravityField();
	if (!GravityField)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("GetUniformGravity failed, native Simulation does not have a gravity field."));
		return FVector(ForceInit);
	}

	agx::UniformGravityField* UniformField = dynamic_cast<agx::UniformGravityField*>(GravityField);
	if (!UniformField)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("GetUniformGravity called on Simulation with a Gravity Field that is not a "
				 "UniformGravityField."));
		return FVector(ForceInit);
	}

	// Convert AGX Dynamics' m/s^2 to Unreal Engine's cm/s^2.
	return ConvertDisplacement(UniformField->getGravity());
}

void FSimulationBarrier::SetPointGravity(const FVector& Origin, float Magnitude)
{
	// Magnitude from cm/s^2 to m/s^2.
	agx::Real MagnitudeAgx = ConvertDistanceToAGX(Magnitude);
	agx::Vec3 OriginAgx = ConvertDisplacement(Origin);

	agx::PointGravityFieldRef Field = new agx::PointGravityField(OriginAgx, MagnitudeAgx);
	NativeRef->Native->setGravityField(Field);
}

FVector FSimulationBarrier::GetPointGravity(float& OutMagnitude) const
{
	agx::GravityField* GravityField = NativeRef->Native->getGravityField();
	if (!GravityField)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("GetPointGravity failed, native Simulation does not have a gravity field."));
		OutMagnitude = 0.f;
		return FVector(ForceInit);
	}

	agx::PointGravityField* PointField = dynamic_cast<agx::PointGravityField*>(GravityField);
	if (!PointField)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("GetPointGravity called on Simulation with a Gravity Field that is not a "
				 "PointGravityField."));
		OutMagnitude = 0.f;
		return FVector(ForceInit);
	}

	OutMagnitude = ConvertToUnreal<float>(PointField->getGravity());
	return ConvertDisplacement(PointField->getCenter());
}

namespace
{
	template <typename T>
	FGuid GetGuid(const T* NativeObject)
	{
		if (!NativeObject)
		{
			FGuid Guid;
			Guid.Invalidate();
			return Guid;
		}

		return Convert(NativeObject->getUuid());
	}
}

TArray<FShapeContactBarrier> FSimulationBarrier::GetShapeContacts(const FShapeBarrier& Shape) const
{
	check(HasNative());
	check(Shape.HasNative());

	// Get the Geometry Contact information from AGX Dynamics.
	agxCollide::GeometryContactPtrVector ContactsAGX;
	agxCollide::Geometry* GeometryAGX = Shape.GetNative()->NativeGeometry;
	NativeRef->Native->getSpace()->getGeometryContacts(ContactsAGX, GeometryAGX);
	size_t NumContacts = ContactsAGX.size();
	// Save one for INVALID_INDEX/InvalidIndex.
	int32 MaxAllowed = std::numeric_limits<int32>::max() - 1;
	if (NumContacts > MaxAllowed)
	{
		UE_LOG(
			LogAGX, Warning, TEXT("Too many ShapeContacts, %zu, will only see the first %d."),
			NumContacts, MaxAllowed);
		NumContacts = MaxAllowed;
	}

	// Wrap each Geometry Contact in a Barrier.
	TArray<FShapeContactBarrier> Contacts;
	Contacts.Reserve(NumContacts);
	for (int32 I = 0; I < NumContacts; ++I)
	{
		agxCollide::GeometryContact* ContactAGX = ContactsAGX[I];

		// We're pessimizing the MaxAllowed limit since any nullptr or invalid Geometry Contacts
		// will still count towards the limit. I don't think this will ever matter, but if it does
		// simply don't truncate NumContacts, let I loop over all Geometry Contacts, and break when
		// the TArray is full.
		if (ContactAGX == nullptr)
		{
			continue;
		}
		if (!ContactAGX->isValid())
		{
			continue;
		}
		Contacts.Add(AGXBarrierFactories::CreateShapeContactBarrier(*ContactAGX));
	}

	return Contacts;
}

TArray<FShapeContactBarrier> FSimulationBarrier::GetShapeContacts() const
{
	check(HasNative());

	TArray<FShapeContactBarrier> ShapeContactBarriers;

	const agxCollide::GeometryContactPtrVector& ContactVectorAGX =
		NativeRef->Native->getSpace()->getGeometryContacts();
	if (ContactVectorAGX.size() == 0)
		return ShapeContactBarriers;

	ShapeContactBarriers.Reserve(ContactVectorAGX.size());
	for (agxCollide::GeometryContact* ContactAGX : ContactVectorAGX)
	{
		if (ContactAGX == nullptr || !ContactAGX->isValid())
			continue;

		ShapeContactBarriers.Add(AGXBarrierFactories::CreateShapeContactBarrier(*ContactAGX));
	}

	return ShapeContactBarriers;
}

void FSimulationBarrier::Step()
{
	check(HasNative());
	try
	{
		NativeRef->Native->stepForward();
	}
	catch (const std::runtime_error& error)
	{
		/// @todo We really need to do something here. Should at least prevent the simulation from
		/// stepping again as that will likely not end well.
		UE_LOG(
			LogAGX, Error,
			TEXT("Got exception from AGX Dynamics. The simulation state is now unreliable. The "
				 "scene should be recreated."));
		UE_LOG(
			LogAGX, Error,
			TEXT("The LogAGXDynamics log category may contain additional information, see either "
				 "the Output Log panel in Unreal Editor or the log file."));
		UE_LOG(LogAGX, Error, TEXT("Error message: %s"), UTF8_TO_TCHAR(error.what()));
	}
}

double FSimulationBarrier::GetTimeStamp() const
{
	check(HasNative());
	return ConvertToUnreal<double>(NativeRef->Native->getTimeStamp());
}

void FSimulationBarrier::SetTimeStamp(double TimeStamp)
{
	check(HasNative());
	NativeRef->Native->setTimeStamp(ConvertToAGX(TimeStamp));
}

void FSimulationBarrier::SetPreIntegratePositions(bool Enable)
{
	check(HasNative());
	NativeRef->Native->setPreIntegratePositions(Enable);
}

bool FSimulationBarrier::GetPreIntegratePositions() const
{
	check(HasNative());
	return NativeRef->Native->getPreIntegratePositions();
}

void FSimulationBarrier::SetNumThreads(uint32 NumThreads)
{
	agx::setNumThreads(static_cast<size_t>(NumThreads));
}

uint32 FSimulationBarrier::GetNumThreads()
{
	return agx::getNumThreads();
}

void FSimulationBarrier::SetStatisticsEnabled(bool bEnabled)
{
	agx::Statistics::instance()->setEnable(bEnabled);
}

namespace
{
	float GetStatisticsTime(void* Instance, const char* Name)
	{
		agx::Statistics::Data<agx::Real>* Entry =
			agx::Statistics::instance()->getData<agx::Real>(Instance, Name);
		if (Entry == nullptr)
		{
			/// @todo Unclear where this message should be. Not here, since the user may not be
			/// interested in this particular part of the statistics.
			// UE_LOG(
			//	LogAGX, Warning, TEXT("Could not get '%s' from AGX Dynamics Statistics."),
			//	UTF8_TO_TCHAR(Name));
			return -1.0f;
		}
		return ConvertToUnreal<float>(Entry->value());
	}

	int32 GetStatisticsCount(void* Instance, const char* Name)
	{
		agx::Statistics::Data<size_t>* Entry =
			agx::Statistics::instance()->getData<size_t>(Instance, Name);
		if (Entry == nullptr)
		{
			return -1;
		}
		size_t ValueAgx = Entry->value();
		const size_t MaxAllowed = std::numeric_limits<int32>::max();
		if (ValueAgx > MaxAllowed)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Statistics value %llu for '%s' is too large, truncated to %d."),
				(unsigned long long) ValueAgx, UTF8_TO_TCHAR(Name), MaxAllowed);
			ValueAgx = MaxAllowed;
		}
		return static_cast<int32>(ValueAgx);
	}
};

FAGX_Statistics FSimulationBarrier::GetStatistics()
{
	FAGX_Statistics Statistics;

	void* SimulationContext = static_cast<void*>(NativeRef->Native.get());
	Statistics.StepForwardTime = GetStatisticsTime(SimulationContext, "Step forward time");
	Statistics.PreCollideTime = GetStatisticsTime(SimulationContext, "Pre-collide event time");
	Statistics.ContactEventsTime =
		GetStatisticsTime(SimulationContext, "Triggering contact events");
	Statistics.PreStepTime = GetStatisticsTime(SimulationContext, "Pre-step event time");
	Statistics.DynamicsSystemTime = GetStatisticsTime(SimulationContext, "Dynamics-system time");
	Statistics.SpaceTime = GetStatisticsTime(SimulationContext, "Collision-detection time");
	Statistics.PostStepTime = GetStatisticsTime(SimulationContext, "Post-step event time");
	Statistics.LastStepTime = GetStatisticsTime(SimulationContext, "Last-step event time");
	Statistics.InterStepTime = GetStatisticsTime(SimulationContext, "Inter-step time");
	Statistics.NumParticles = GetStatisticsCount(SimulationContext, "Num particles");

	void* DynamicsContext = static_cast<void*>(NativeRef->Native.get()->getDynamicsSystem());
	Statistics.NumBodies = GetStatisticsCount(DynamicsContext, "Num enabled rigid bodies");
	Statistics.NumConstraints = GetStatisticsCount(DynamicsContext, "Num binary constraints") +
								GetStatisticsCount(DynamicsContext, "Num multi-body constraints");
	Statistics.NumContacts = GetStatisticsCount(DynamicsContext, "Num contact constraints");

	return Statistics;
}

bool FSimulationBarrier::HasNative() const
{
	return NativeRef != nullptr && NativeRef->Native != nullptr;
}

void FSimulationBarrier::AllocateNative()
{
	NativeRef = std::make_unique<FSimulationRef>(new agxSDK::Simulation());
}

FSimulationRef* FSimulationBarrier::GetNative()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return NativeRef.get();
}

const FSimulationRef* FSimulationBarrier::GetNative() const
{
	if (!HasNative())
	{
		return nullptr;
	}
	return NativeRef.get();
}

void FSimulationBarrier::ReleaseNative()
{
	NativeRef = nullptr;
}

FShapeContactMergeSplitThresholdsBarrier FSimulationBarrier::GetGlobalShapeContactTresholds() const
{
	check(HasNative());
	return FShapeContactMergeSplitThresholdsBarrier(std::make_unique<FMergeSplitThresholdsRef>(
		NativeRef->Native->getMergeSplitHandler()->getGlobalContactThresholds()));
}

FConstraintMergeSplitThresholdsBarrier FSimulationBarrier::GetGlobalConstraintTresholds() const
{
	check(HasNative());
	return FConstraintMergeSplitThresholdsBarrier(std::make_unique<FMergeSplitThresholdsRef>(
		NativeRef->Native->getMergeSplitHandler()->getGlobalConstraintThresholds()));
}

FWireMergeSplitThresholdsBarrier FSimulationBarrier::GetGlobalWireTresholds() const
{
	check(HasNative());
	return FWireMergeSplitThresholdsBarrier(std::make_unique<FMergeSplitThresholdsRef>(
		NativeRef->Native->getMergeSplitHandler()->getGlobalWireThresholds()));
}
