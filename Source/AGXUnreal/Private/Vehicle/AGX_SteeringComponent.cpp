// Copyright 2025, Algoryx Simulation AB.

#include "Vehicle/AGX_SteeringComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGX_Simulation.h"
#include "Import/AGX_ImportContext.h"
#include "Vehicle/AGX_AckermannSteeringParameters.h"
#include "Vehicle/AGX_BellCrankSteeringParameters.h"
#include "Vehicle/AGX_DavisSteeringParameters.h"
#include "Vehicle/AGX_RackPinionSteeringParameters.h"
#include "Vehicle/AGX_WheelJointComponent.h"
#include "Vehicle/WheelJointBarrier.h"
#include "Utilities/AGX_ImportRuntimeUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

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

void UAGX_SteeringComponent::SetSteeringAngle(double Angle)
{
	if (!HasNative())
		return;

	NativeBarrier.SetSteeringAngle(Angle);
}

double UAGX_SteeringComponent::GetSteeringAngle() const
{
	if (!HasNative())
		return 0.0;

	return NativeBarrier.GetSteeringAngle();
}

void UAGX_SteeringComponent::CopyFrom(const FSteeringBarrier& Barrier, FAGX_ImportContext* Context)
{
	ImportGuid = Barrier.GetGuid();
	ImportName = Barrier.GetName(); // Unmodifiled AGX name.
	bEnabled = Barrier.GetEnabled();

	const FString CleanBarrierName =
		FAGX_ImportRuntimeUtilities::RemoveModelNameFromBarrierName(Barrier.GetName(), Context);
	const FString Name = FAGX_ObjectUtilities::SanitizeAndMakeNameUnique(
		GetOuter(), CleanBarrierName, UAGX_ConstraintComponent::StaticClass());
	Rename(*Name);

	// TODO
	/*if (Context != nullptr && Context->Constraints != nullptr)
	{
		// find wheeljoints and assign them here.
	}*/
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

namespace AGX_SteeringComponent_helpers
{
	void SetLocalScope(UAGX_SteeringComponent& Component)
	{
		AActor* const Owner = FAGX_ObjectUtilities::GetRootParentActor(Component);
		Component.LeftWheelJoint.LocalScope = Owner;
		Component.RightWheelJoint.LocalScope = Owner;
	}
}

void UAGX_SteeringComponent::OnRegister()
{
	Super::OnRegister();

	// On Register is called after all object initialization has completed, i.e. Unreal Engine
	// will not be messing with this object anymore. It is now safe to set the Local Scope on our
	// Component References.
	AGX_SteeringComponent_helpers::SetLocalScope(*this);
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

void UAGX_SteeringComponent::CreateNative()
{
	auto CreateNativeFailNotification = [this]()
	{
		const FString Text = FString::Printf(
			TEXT("Could not create native for Steerin Component '%s' in '%s'. The "
				 "Output Log may include more information."),
			*GetName(), *GetLabelSafe(GetOwner()));
		FAGX_NotificationUtilities::ShowNotification(Text, SNotificationItem::CS_Fail);
	};

	auto LeftWheelComp = LeftWheelJoint.GetWheelJointComponent();
	FWheelJointBarrier* LeftWheelBarrier =
		LeftWheelComp != nullptr
			? static_cast<FWheelJointBarrier*>(LeftWheelComp->GetOrCreateNative())
			: nullptr;
	if (LeftWheelBarrier == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Steering Component '%s' in '%s' does not have a valid left wheel selected."),
			*GetName(), *GetLabelSafe(GetOwner()));
		CreateNativeFailNotification();
		return;
	}

	auto RightWheelComp = RightWheelJoint.GetWheelJointComponent();
	FWheelJointBarrier* RightWheelBarrier =
		RightWheelComp != nullptr
			? static_cast<FWheelJointBarrier*>(RightWheelComp->GetOrCreateNative())
			: nullptr;
	if (RightWheelBarrier == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Steering Component '%s' in '%s' does not have a valid right wheel selected."),
			*GetName(), *GetLabelSafe(GetOwner()));
		CreateNativeFailNotification();
		return;
	}

	if (SteeringParameters == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Steering Component '%s' in '%s' does not have a valid Steering Parameters asset "
				 "selected."),
			*GetName(), *GetLabelSafe(GetOwner()));
		CreateNativeFailNotification();
		return;
	}

	if (SteeringParameters->IsA<UAGX_AckermannSteeringParameters>())
	{
		NativeBarrier.AllocateAckermann(
			*LeftWheelBarrier, *RightWheelBarrier, SteeringParameters->SteeringData);
	}
	else if (SteeringParameters->IsA<UAGX_BellCrankSteeringParameters>())

	{
		NativeBarrier.AllocateBellCrank(
			*LeftWheelBarrier, *RightWheelBarrier, SteeringParameters->SteeringData);
	}
	else if (SteeringParameters->IsA<UAGX_DavisSteeringParameters>())
	{
		NativeBarrier.AllocateDavis(
			*LeftWheelBarrier, *RightWheelBarrier, SteeringParameters->SteeringData);
	}
	else if (SteeringParameters->IsA<UAGX_RackPinionSteeringParameters>())
	{
		NativeBarrier.AllocateRackPinion(
			*LeftWheelBarrier, *RightWheelBarrier, SteeringParameters->SteeringData);
	}
	else
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Steering Component '%s' in '%s' has an unknown Steering Parameters asset "
				 "type selected."),
			*GetName(), *GetLabelSafe(GetOwner()));
		CreateNativeFailNotification();
		return;
	}

	AGX_CHECK(HasNative());
	UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(this);
	if (Sim != nullptr)
		Sim->Add(*this);
}

bool UAGX_SteeringComponent::GetEnabled() const
{
	return IsEnabled();
}
