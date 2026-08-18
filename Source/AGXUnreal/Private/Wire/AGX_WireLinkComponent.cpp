// Copyright 2026, Algoryx Simulation AB.

#include "Wire/AGX_WireLinkComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_NativeOwnerSceneComponentInstanceData.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_RigidBodyComponent.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_StringUtilities.h"
#include "Wire/AGX_WireComponent.h"

// Unreal Engine includes.
#include "CoreGlobals.h"
#include "Engine/Level.h"
#include "GameFramework/Actor.h"

UAGX_WireLinkComponent::UAGX_WireLinkComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bWantsOnUpdateTransform = true;
}

UAGX_RigidBodyComponent* UAGX_WireLinkComponent::GetRigidBody() const
{
	return FAGX_ObjectUtilities::FindFirstAncestorOfType<UAGX_RigidBodyComponent>(*this);
}

TArray<UAGX_WireComponent*> UAGX_WireLinkComponent::GetConnectedWires() const
{
	TArray<UAGX_WireComponent*> Wires;
	if (!HasNative())
		return Wires;

	const TArray<FWireBarrier> ConnectedWireBarriers = NativeBarrier.GetConnectedWires();
	if (ConnectedWireBarriers.IsEmpty())
		return Wires;

	TSet<FGuid> ConnectedWireGuids;
	ConnectedWireGuids.Reserve(ConnectedWireBarriers.Num());
	for (const FWireBarrier& ConnectedWireBarrier : ConnectedWireBarriers)
	{
		ConnectedWireGuids.Add(ConnectedWireBarrier.GetGuid());
	}

	const AActor* Owner = GetOwner();
	const ULevel* Level = Owner != nullptr ? Owner->GetLevel() : nullptr;
	if (Level == nullptr)
		return Wires;

	for (AActor* Actor : Level->Actors)
	{
		if (Actor == nullptr)
			continue;

		TArray<UAGX_WireComponent*> WireComponents;
		Actor->GetComponents<UAGX_WireComponent>(WireComponents, false);
		for (UAGX_WireComponent* WireComponent : WireComponents)
		{
			const FWireBarrier* WireBarrier =
				WireComponent != nullptr ? WireComponent->GetNative() : nullptr;
			if (WireBarrier == nullptr)
				continue;

			if (ConnectedWireGuids.Contains(WireBarrier->GetGuid()))
				Wires.AddUnique(WireComponent);
		}
	}

	return Wires;
}

void UAGX_WireLinkComponent::SetBendStiffness(double InBendStiffness)
{
	BendStiffness = InBendStiffness;
	if (HasNative())
		NativeBarrier.SetWireConnectionBendStiffness(BendStiffness.Value);
}

double UAGX_WireLinkComponent::GetBendStiffness() const
{
	return BendStiffness.Value;
}

void UAGX_WireLinkComponent::SetTwistStiffness(double InTwistStiffness)
{
	TwistStiffness = InTwistStiffness;
	if (HasNative())
		NativeBarrier.SetWireConnectionTwistStiffness(TwistStiffness.Value);
}

double UAGX_WireLinkComponent::GetTwistStiffness() const
{
	return TwistStiffness.Value;
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
		const FString Message = FString::Printf(
			TEXT("UAGX_WireLinkComponent '%s' in '%s': Failed to create native agxWire::Link. "
				 "Check that this component is attached to a UAGX_RigidBodyComponent, "
				 "and that the AgX-WireLink license module is active."),
			*GetName(), *GetLabelSafe(GetOwner()));
		FAGX_NotificationUtilities::ShowNotification(Message, SNotificationItem::CS_Fail);
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

void UAGX_WireLinkComponent::OnUpdateTransform(
	EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	Super::OnUpdateTransform(UpdateTransformFlags, Teleport);
	if (GetRigidBody() != nullptr && !GetRelativeTransform().Equals(FTransform::Identity))
		SetRelativeTransform(FTransform::Identity);
}

void UAGX_WireLinkComponent::OnAttachmentChanged()
{
	Super::OnAttachmentChanged();
	if (GetRigidBody() != nullptr && !GetRelativeTransform().Equals(FTransform::Identity))
		SetRelativeTransform(FTransform::Identity);
}

#if WITH_EDITOR
void UAGX_WireLinkComponent::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_WireLinkComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_WireLinkComponent::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
		return;

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireLinkComponent, BendStiffness),
		[](ThisClass* This) { This->SetBendStiffness(This->BendStiffness.Value); });
	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_WireLinkComponent, TwistStiffness),
		[](ThisClass* This) { This->SetTwistStiffness(This->TwistStiffness.Value); });
}
#endif

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
		const FString Message = FString::Printf(
			TEXT("UAGX_WireLinkComponent '%s' in '%s': Cannot create native — this component "
				 "must be attached as a child of the UAGX_RigidBodyComponent it wraps."),
			*GetName(), *GetLabelSafe(GetOwner()));
		FAGX_NotificationUtilities::ShowNotification(Message, SNotificationItem::CS_Fail);
		return;
	}

	FRigidBodyBarrier* BodyBarrier = BodyComponent->GetOrCreateNative();
	if (BodyBarrier == nullptr || !BodyBarrier->HasNative())
	{
		const FString Message = FString::Printf(
			TEXT("UAGX_WireLinkComponent '%s' in '%s': Cannot create native — the attached "
				 "body '%s' does not have a native AGX rigid body."),
			*GetName(), *GetLabelSafe(GetOwner()), *BodyComponent->GetName());
		FAGX_NotificationUtilities::ShowNotification(Message, SNotificationItem::CS_Fail);
		return;
	}

	NativeBarrier.AllocateNative(*BodyBarrier);

	if (!HasNative())
	{
		const FString Message = FString::Printf(
			TEXT("UAGX_WireLinkComponent '%s' in '%s': FWireLinkBarrier::AllocateNative "
				 "succeeded but HasNative() is still false. The AgX-WireLink license module "
				 "may be missing from the active license."),
			*GetName(), *GetLabelSafe(GetOwner()));
		FAGX_NotificationUtilities::ShowNotification(Message, SNotificationItem::CS_Fail);
		return;
	}

	UpdateNativeProperties();
}

void UAGX_WireLinkComponent::UpdateNativeProperties()
{
	NativeBarrier.SetWireConnectionBendStiffness(BendStiffness.Value);
	NativeBarrier.SetWireConnectionTwistStiffness(TwistStiffness.Value);
}
