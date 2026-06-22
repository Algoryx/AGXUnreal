// Copyright 2026, Algoryx Simulation AB.

#include "Wire/AGX_WireLinkComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_NativeOwnerSceneComponentInstanceData.h"
#include "AGX_RigidBodyComponent.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "CoreGlobals.h"

UAGX_WireLinkComponent::UAGX_WireLinkComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

UAGX_RigidBodyComponent* UAGX_WireLinkComponent::GetRigidBody() const
{
	return FAGX_ObjectUtilities::FindFirstAncestorOfType<UAGX_RigidBodyComponent>(*this);
}

void UAGX_WireLinkComponent::RegisterConnectedWire(UAGX_WireComponent* Wire)
{
	if (Wire == nullptr)
	{
		return;
	}

	// Deduplicate.
	for (UAGX_WireComponent* const Existing : ConnectedWires)
	{
		if (Existing == Wire)
		{
			return;
		}
	}

	ConnectedWires.Add(Wire);
}

bool UAGX_WireLinkComponent::HasNative() const
{
	return NativeBarrier.HasNative();
}

uint64 UAGX_WireLinkComponent::GetNativeAddress() const
{
	return static_cast<uint64>(NativeBarrier.GetNativeAddress());
}

void UAGX_WireLinkComponent::SetNativeAddress(uint64 NativeAddress)
{
	check(!HasNative());
	NativeBarrier.SetNativeAddress(static_cast<uintptr_t>(NativeAddress));
}

void UAGX_WireLinkComponent::BeginPlay()
{
	Super::BeginPlay();

	if (HasNative())
	{
		// Native was inherited from a Blueprint reconstruction — nothing more to do.
		// The link activates implicitly when its connected wires are added to the simulation.
		return;
	}

	if (GIsReconstructingBlueprintInstances)
	{
		return;
	}

	GetOrCreateNative();

	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("UAGX_WireLinkComponent '%s' in '%s': Failed to create native agxWire::Link. "
				 "Check that this component is attached to a UAGX_RigidBodyComponent, "
				 "and that the AgX-WireLink license module is active."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}
}

void UAGX_WireLinkComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	if (!GIsReconstructingBlueprintInstances)
	{
		if (HasNative())
		{
			NativeBarrier.ReleaseNative();
		}

		ConnectedWires.Empty();
	}
	// If GIsReconstructingBlueprintInstances, another WireLinkComponent will inherit
	// the native via Component Instance Data — do not release it here.

	Super::EndPlay(Reason);
}

TStructOnScope<FActorComponentInstanceData>
UAGX_WireLinkComponent::GetComponentInstanceData() const
{
	return MakeStructOnScope<FActorComponentInstanceData,
							 FAGX_NativeOwnerSceneComponentInstanceData>(
		this, this,
		[](UActorComponent* Component) -> IAGX_NativeOwner*
		{ return Cast<UAGX_WireLinkComponent>(Component); });
}

FWireLinkBarrier* UAGX_WireLinkComponent::GetNative()
{
	return HasNative() ? &NativeBarrier : nullptr;
}

const FWireLinkBarrier* UAGX_WireLinkComponent::GetNative() const
{
	return HasNative() ? &NativeBarrier : nullptr;
}

FWireLinkBarrier* UAGX_WireLinkComponent::GetOrCreateNative()
{
	if (HasNative())
	{
		return &NativeBarrier;
	}

	checkf(
		!GIsReconstructingBlueprintInstances,
		TEXT("UAGX_WireLinkComponent::GetOrCreateNative called while Blueprint reconstruction is "
			 "in progress. The native should be inherited via Component Instance Data."));

	CreateNative();

	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("UAGX_WireLinkComponent '%s' in '%s': GetOrCreateNative could not create a "
				 "native. Ensure this component is attached to a UAGX_RigidBodyComponent, "
				 "and that the AgX-WireLink module is licensed."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return nullptr;
	}

	return &NativeBarrier;
}

void UAGX_WireLinkComponent::CreateNative()
{
	check(!HasNative());
	check(!GIsReconstructingBlueprintInstances);

	// Resolve the body via the attachment hierarchy.
	UAGX_RigidBodyComponent* BodyComponent = GetRigidBody();
	if (BodyComponent == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("UAGX_WireLinkComponent '%s' in '%s': Cannot create native — this component "
				 "must be attached as a child of the UAGX_RigidBodyComponent it wraps."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	FRigidBodyBarrier* BodyBarrier = BodyComponent->GetOrCreateNative();
	if (BodyBarrier == nullptr || !BodyBarrier->HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("UAGX_WireLinkComponent '%s' in '%s': Cannot create native — the attached "
				 "body '%s' does not have a native AGX rigid body."),
			*GetName(), *GetLabelSafe(GetOwner()), *BodyComponent->GetName());
		return;
	}

	NativeBarrier.AllocateNative(*BodyBarrier);

	if (!HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("UAGX_WireLinkComponent '%s' in '%s': FWireLinkBarrier::AllocateNative "
				 "succeeded but HasNative() is still false. The AgX-WireLink license module "
				 "may be missing from the active license."),
			*GetName(), *GetLabelSafe(GetOwner()));
	}
}
