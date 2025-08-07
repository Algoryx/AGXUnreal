// Copyright 2025, Algoryx Simulation AB.

#include "Sensors/AGX_IMUSensorComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_NativeOwnerInstanceData.h"
#include "AGX_PropertyChangedDispatcher.h"

#define LOCTEXT_NAMESPACE "AGX_IMUSensor"

UAGX_IMUSensorComponent::UAGX_IMUSensorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAGX_IMUSensorComponent::SetEnabled(bool InEnabled)
{
	bEnabled = InEnabled;

	if (HasNative())
		NativeBarrier.SetEnabled(InEnabled);
}

bool UAGX_IMUSensorComponent::GetEnabled() const
{
	if (HasNative())
		return NativeBarrier.GetEnabled();

	return bEnabled;
}

#if WITH_EDITOR
bool UAGX_IMUSensorComponent::CanEditChange(const FProperty* InProperty) const
{
	const bool SuperCanEditChange = Super::CanEditChange(InProperty);
	if (!SuperCanEditChange)
		return false;

	if (InProperty == nullptr)
	{
		return SuperCanEditChange;
	}

	const bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
	if (bIsPlaying)
	{
		// List of names of properties that does not support editing after initialization.
		static const TArray<FName> PropertiesNotEditableDuringPlay = {
			GET_MEMBER_NAME_CHECKED(ThisClass, bUseAccelerometer),
			GET_MEMBER_NAME_CHECKED(ThisClass, bUseGyroscope),
			GET_MEMBER_NAME_CHECKED(ThisClass, bUseMagnetometer)};

		if (PropertiesNotEditableDuringPlay.Contains(InProperty->GetFName()))
			return false;
	}

	return SuperCanEditChange;
}

void UAGX_IMUSensorComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that this object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_IMUSensorComponent::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_IMUSensorComponent::UpdateNativeProperties()
{
	AGX_CHECK(HasNative());

	NativeBarrier.SetEnabled(bEnabled);
}

void UAGX_IMUSensorComponent::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
		return;

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_IMUSensorComponent, bEnabled),
		[](ThisClass* This) { This->SetEnabled(This->bEnabled); });
}

#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE

void UAGX_IMUSensorComponent::UpdateNativeTransform()
{
	if (HasNative())
		NativeBarrier.SetTransform(GetComponentTransform());
}

FIMUBarrier* UAGX_IMUSensorComponent::GetOrCreateNative()
{
	if (HasNative())
		return GetNative();

	NativeBarrier.AllocateNative();
	if (HasNative())
		UpdateNativeProperties();

	return GetNative();
}

FIMUBarrier* UAGX_IMUSensorComponent::GetNative()
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

const FIMUBarrier* UAGX_IMUSensorComponent::GetNative() const
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

bool UAGX_IMUSensorComponent::HasNative() const
{
	return NativeBarrier.HasNative();
}

uint64 UAGX_IMUSensorComponent::GetNativeAddress() const
{
	if (!HasNative())
		return 0;

	NativeBarrier.IncrementRefCount();
	return NativeBarrier.GetNativeAddress();
}

void UAGX_IMUSensorComponent::SetNativeAddress(uint64 NativeAddress)
{
	check(!HasNative());
	NativeBarrier.SetNativeAddress(NativeAddress);
	NativeBarrier.DecrementRefCount();
}

void UAGX_IMUSensorComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAGX_IMUSensorComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);

	if (HasNative())
		NativeBarrier.ReleaseNative();
}

TStructOnScope<FActorComponentInstanceData> UAGX_IMUSensorComponent::GetComponentInstanceData()
	const
{
	return MakeStructOnScope<FActorComponentInstanceData, FAGX_NativeOwnerInstanceData>(
		this, this,
		[](UActorComponent* Component)
		{
			ThisClass* AsThisClass = Cast<ThisClass>(Component);
			return static_cast<IAGX_NativeOwner*>(AsThisClass);
		});
}
