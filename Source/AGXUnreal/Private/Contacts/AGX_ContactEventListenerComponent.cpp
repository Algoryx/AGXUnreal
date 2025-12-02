// Copyright 2025, Algoryx Simulation AB.

#include "Contacts/AGX_ContactEventListenerComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_Simulation.h"
#include "Contacts/ContactListenerBarrier.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Shapes/AnyShapeBarrier.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "Engine/World.h"

void UAGX_ContactEventListenerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Create an AGX Dynamics Contact Event Listener that calls our ImpactCallback, ContactCallback,
	// and SeparationCallback member functions via lambda functions.
	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(this);
	FSimulationBarrier* SimulationBarrier = Simulation->GetNative();
	CreateContactEventListener(
		*SimulationBarrier,
		[this](double TimeStamp, FShapeContactBarrier& ShapeContact)
		{ return ImpactCallback(TimeStamp, ShapeContact); },
		[this](double TimeStamp, FShapeContactBarrier& ShapeContact)
		{ return ContactCallback(TimeStamp, ShapeContact); },
		[this](double TimeStamp, FAnyShapeBarrier& FirstShape, FAnyShapeBarrier& SecondShape)
		{ SeparationCallback(TimeStamp, FirstShape, SecondShape); });
}

EAGX_KeepContactPolicy UAGX_ContactEventListenerComponent::ImpactCallback(
	double TimeStamp, FShapeContactBarrier& ContactBarrier)
{
	// Called during Step Forward by the AGX Dynamics Contact Event Listener. Forward to the
	// Blueprint function and the delegate.
	FAGX_ShapeContact Contact(ContactBarrier);
	EAGX_KeepContactPolicy Policy = Impact(TimeStamp, Contact);
	if (Policy != EAGX_KeepContactPolicy::RemoveContactImmediately)
	{
		FAGX_KeepContactPolicyHandle PolicyHandle {&Policy};
		OnImpact.Broadcast(TimeStamp, Contact, PolicyHandle);
	}
	return Policy;
}

EAGX_KeepContactPolicy UAGX_ContactEventListenerComponent::ContactCallback(
	double TimeStamp, FShapeContactBarrier& ContactBarrier)
{
	// Called during Step Forward by the AGX Dynamics Contact Event Listener. Forward to the
	// Blueprint function and the delegate
	FAGX_ShapeContact ContactUnreal(ContactBarrier);
	EAGX_KeepContactPolicy Policy = Contact(TimeStamp, ContactUnreal);
	if (Policy != EAGX_KeepContactPolicy::RemoveContactImmediately)
	{
		FAGX_KeepContactPolicyHandle PolicyHandle {&Policy};
		OnContact.Broadcast(TimeStamp, ContactUnreal, PolicyHandle);
	}
	return Policy;
}

namespace AGX_ContactEventListenerComponent_helpers
{
	bool IsMatch(UAGX_ShapeComponent* Shape, const FGuid& Guid)
	{
		return Shape->GetNative()->GetGeometryGuid() == Guid;
	}

	template <typename UComponent>
	UComponent* FindComponentByGuid(const FGuid& Guid, UWorld& World)
	{
		return FAGX_ObjectUtilities::FindComponentByPredicate<UComponent>(
			World, [&Guid](UComponent* Component) { return IsMatch(Component, Guid); });
	}
}

void UAGX_ContactEventListenerComponent::SeparationCallback(
	double TimeStamp, FAnyShapeBarrier& FirstShapeBarrier, FAnyShapeBarrier& SecondShapeBarrier)
{
	using namespace AGX_ContactEventListenerComponent_helpers;

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Contact Event Listener '%s' in '%s' cannot find Shape Components because it does "
				 "not have a world."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	OnSeparationBarrier.Broadcast(TimeStamp, FirstShapeBarrier, SecondShapeBarrier);

	UAGX_ShapeComponent* FirstShape =
		FindComponentByGuid<UAGX_ShapeComponent>(FirstShapeBarrier.GetGeometryGuid(), *World);
	UAGX_ShapeComponent* SecondShape =
		FindComponentByGuid<UAGX_ShapeComponent>(SecondShapeBarrier.GetGeometryGuid(), *World);

	Separation(TimeStamp, FirstShape, SecondShape);
	OnSeparation.Broadcast(TimeStamp, FirstShape, SecondShape);
}

EAGX_KeepContactPolicy UAGX_ContactEventListenerComponent::Impact_Implementation(
	double TimeStamp, const FAGX_ShapeContact& ShapeContact)
{
	return EAGX_KeepContactPolicy::KeepContact;
}

EAGX_KeepContactPolicy UAGX_ContactEventListenerComponent::Contact_Implementation(
	double TimeStamp, const FAGX_ShapeContact& ShapeContact)
{
	return EAGX_KeepContactPolicy::KeepContact;
}

void UAGX_ContactEventListenerComponent::Separation_Implementation(
	double TimeStamp, const UAGX_ShapeComponent* FirstShape, UAGX_ShapeComponent* SecondShape)
{
	// Nothing to do.
}

void UAGX_ContactEventListenerComponent::SeparationBarrier_Implementation(
	double TimeStamp, const FAnyShapeBarrier& FirstShape, const FAnyShapeBarrier& SecondShape)
{
	// Nothing to do.
}
