// Copyright 2025, Algoryx Simulation AB.

#include "Vehicle/AGX_SteeringComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_NativeOwnerInstanceData.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_Simulation.h"

void UAGX_SteeringComponent::SetEnabled(bool InEnabled)
{
	if (HasNative())
	{
		NativeBarrier.SetEnabled(InEnabled);
	}

	bEnabled = InEnabled;
}

bool UAGX_SteeringComponent::IsEnabled() const
{
	if (HasNative())
	{
		return NativeBarrier.GetEnabled();
	}

	return bEnabled;
}

void UAGX_SteeringComponent::CopyFrom(const FSteeringBarrier& Barrier, FAGX_ImportContext* Context)
{
	// TODO
}

FSteeringBarrier* UAGX_SteeringComponent::GetNative()
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

const FSteeringBarrier* UAGX_SteeringComponent::GetNative() const
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

bool UAGX_SteeringComponent::HasNative() const
{
	return NativeBarrier.HasNative();
}

uint64 UAGX_SteeringComponent::GetNativeAddress() const
{
	return static_cast<uint64>(NativeBarrier.GetNativeAddress());
}

void UAGX_SteeringComponent::SetNativeAddress(uint64 NativeAddress)
{
	check(!HasNative());
	NativeBarrier.SetNativeAddress(static_cast<uintptr_t>(NativeAddress));
}

void UAGX_SteeringComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!HasNative() && !GIsReconstructingBlueprintInstances)
		CreateNative();
}

void UAGX_SteeringComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);
	if (GIsReconstructingBlueprintInstances)
	{
		// Another UAGX_ObserverFrameComponent will inherit this one's Native, so don't wreck it.
		// The call to NativeBarrier.ReleaseNative below is safe because the AGX Dynamics Simulation
		// will retain a reference counted pointer to the AGX Dynamics Observer Frame.
	}
	else if (
		HasNative() && Reason != EEndPlayReason::EndPlayInEditor &&
		Reason != EEndPlayReason::Quit && Reason != EEndPlayReason::LevelTransition)
	{
		if (UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(this))
			Sim->Remove(*this);
	}

	if (HasNative())
		NativeBarrier.ReleaseNative();
}

TStructOnScope<FActorComponentInstanceData> UAGX_SteeringComponent::GetComponentInstanceData() const
{
	return MakeStructOnScope<FActorComponentInstanceData, FAGX_NativeOwnerInstanceData>(
		this, this,
		[](UActorComponent* Component)
		{
			ThisClass* AsThisClass = Cast<ThisClass>(Component);
			return static_cast<IAGX_NativeOwner*>(AsThisClass);
		});
}

#if WITH_EDITOR
bool UAGX_SteeringComponent::CanEditChange(const FProperty* InProperty) const
{
	const bool SuperCanEditChange = Super::CanEditChange(InProperty);
	if (!SuperCanEditChange)
		return false;

	if (InProperty == nullptr)
		return SuperCanEditChange;

	const bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
	if (bIsPlaying)
	{
		// List of names of properties that does not support editing after initialization.
		static const TArray<FName> PropertiesNotEditableDuringPlay = {
			AGX_MEMBER_NAME(LeftWheelJoint), AGX_MEMBER_NAME(RightWheelJoint)};

		if (PropertiesNotEditableDuringPlay.Contains(InProperty->GetFName()))
			return false;
	}

	return SuperCanEditChange;
}

void UAGX_SteeringComponent::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_SteeringComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_SteeringComponent::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
		return;

	AGX_COMPONENT_DEFAULT_DISPATCHER_BOOL(Enabled);
}
#endif // WITH_EDITOR

void UAGX_SteeringComponent::UpdateNativeProperties()
{
	// TODO
}

void UAGX_SteeringComponent::CreateNative()
{
	// TODO
}

bool UAGX_SteeringComponent::GetEnabled() const
{
	return IsEnabled();
}
