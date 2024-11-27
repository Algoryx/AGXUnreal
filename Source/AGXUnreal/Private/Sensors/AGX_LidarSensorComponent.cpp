// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_LidarSensorComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_NativeOwnerInstanceData.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_Simulation.h"
#include "Sensors/AGX_LidarOutputBase.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_SensorUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "Engine/World.h"
#include "NiagaraComponent.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "NiagaraFunctionLibrary.h"

// Standard library includes.
#include <algorithm>
#include <limits>

#define LOCTEXT_NAMESPACE "AGX_LidarSensor"

UAGX_LidarSensorComponent::UAGX_LidarSensorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	static const TCHAR* DefaultNiagaraSystem =
		TEXT("NiagaraSystem'/AGXUnreal/Sensor/Lidar/NS_LidarNiagaraSystem.NS_LidarNiagaraSystem'");
	NiagaraSystemAsset =
		FAGX_ObjectUtilities::GetAssetFromPath<UNiagaraSystem>(DefaultNiagaraSystem);
}

void UAGX_LidarSensorComponent::SetModel(EAGX_LidarModel InModel)
{
	if (HasBegunPlay())
	{
		FAGX_NotificationUtilities::ShowNotification(
			FString::Printf(
				TEXT("Set Model was called after BeginPlay on Lidar Sensor '%s' in '%s'. This "
					 "function may only be called before BeginPlay."),
				*GetName(), *GetLabelSafe(GetOwner())),
			SNotificationItem::CS_Fail);
		return;
	}

	Model = InModel;
}

EAGX_LidarModel UAGX_LidarSensorComponent::GetModel() const
{
	return Model;
}

void UAGX_LidarSensorComponent::SetEnabled(bool InEnabled)
{
	bEnabled = InEnabled;

	if (HasNative())
		NativeBarrier.SetEnabled(InEnabled);
}

bool UAGX_LidarSensorComponent::GetEnabled() const
{
	if (HasNative())
		return NativeBarrier.GetEnabled();

	return bEnabled;
}

void UAGX_LidarSensorComponent::SetRange(FAGX_RealInterval InRange)
{
	Range = InRange;

	if (HasNative())
		NativeBarrier.SetRange(InRange);
}

FAGX_RealInterval UAGX_LidarSensorComponent::GetRange() const
{
	if (HasNative())
		return NativeBarrier.GetRange();

	return Range;
}

void UAGX_LidarSensorComponent::SetBeamDivergence(double InBeamDivergence)
{
	BeamDivergence = InBeamDivergence;

	if (HasNative())
		NativeBarrier.SetBeamDivergence(InBeamDivergence);
}

double UAGX_LidarSensorComponent::GetBeamDivergence() const
{
	if (HasNative())
		return NativeBarrier.GetBeamDivergence();

	return BeamDivergence;
}

void UAGX_LidarSensorComponent::SetBeamExitRadius(double InBeamExitRadius)
{
	BeamExitRadius = InBeamExitRadius;

	if (HasNative())
		NativeBarrier.SetBeamExitRadius(InBeamExitRadius);
}

double UAGX_LidarSensorComponent::GetBeamExitRadius() const
{
	if (HasNative())
		return NativeBarrier.GetBeamExitRadius();

	return BeamExitRadius;
}

void UAGX_LidarSensorComponent::SetRaytraceDepth(int32 Depth)
{
	RaytraceDepth = Depth;

	if (Depth < 0)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UAGX_LidarSensorComponent::SetRaytraceDepth called with invalid Depth: "
				 "%d. Depth must not be negative."),
			Depth);
		return;
	}

	if (HasNative())
		NativeBarrier.SetRaytraceDepth(static_cast<size_t>(Depth));
}

int32 UAGX_LidarSensorComponent::GetRaytraceDepth() const
{
	if (HasNative())
	{
		return static_cast<int32>(std::min(
			NativeBarrier.GetRaytraceDepth(),
			static_cast<size_t>(std::numeric_limits<int32>::max())));
	}

	return RaytraceDepth;
}

void UAGX_LidarSensorComponent::SetEnableRemovePointsMisses(bool bEnable)
{
	bEnableRemovePointsMisses = bEnable;

	if (HasNative())
		NativeBarrier.SetEnableRemoveRayMisses(bEnable);
}

bool UAGX_LidarSensorComponent::GetEnableRemovePointsMisses() const
{
	if (HasNative())
		return NativeBarrier.GetEnableRemoveRayMisses();

	return bEnableRemovePointsMisses;
}

void UAGX_LidarSensorComponent::SetEnableDistanceGaussianNoise(bool bEnable)
{
	bEnableDistanceGaussianNoise = bEnable;

	if (HasNative())
	{
		if (bEnable)
		{
			NativeBarrier.EnableOrUpdateDistanceGaussianNoise(DistanceNoiseSettings);
		}
		else
		{
			NativeBarrier.DisableDistanceGaussianNoise();
		}
	}
}

bool UAGX_LidarSensorComponent::GetEnableDistanceGaussianNoise() const
{
	if (HasNative())
		return NativeBarrier.GetEnableDistanceGaussianNoise();

	return bEnableDistanceGaussianNoise;
}

void UAGX_LidarSensorComponent::SetDistanceNoiseSettings(
	FAGX_DistanceGaussianNoiseSettings Settings)
{
	DistanceNoiseSettings = Settings;

	if (HasNative() && NativeBarrier.GetEnableDistanceGaussianNoise())
		NativeBarrier.EnableOrUpdateDistanceGaussianNoise(Settings);
}

FAGX_DistanceGaussianNoiseSettings UAGX_LidarSensorComponent::GetDistanceNoiseSettings() const
{
	return DistanceNoiseSettings;
}

void UAGX_LidarSensorComponent::SetEnableRayAngleGaussianNoise(bool bEnable)
{
	bEnableRayAngleGaussianNoise = bEnable;

	if (HasNative())
	{
		if (bEnable)
		{
			NativeBarrier.EnableOrUpdateRayAngleGaussianNoise(RayAngleNoiseSettings);
		}
		else
		{
			NativeBarrier.DisableRayAngleGaussianNoise();
		}
	}
}

bool UAGX_LidarSensorComponent::GetEnableRayAngleGaussianNoise() const
{
	if (HasNative())
		return NativeBarrier.GetEnableRayAngleGaussianNoise();

	return bEnableRayAngleGaussianNoise;
}

void UAGX_LidarSensorComponent::SetRayAngleNoiseSettings(
	FAGX_RayAngleGaussianNoiseSettings Settings)
{
	RayAngleNoiseSettings = Settings;

	if (HasNative() && NativeBarrier.GetEnableRayAngleGaussianNoise())
		NativeBarrier.EnableOrUpdateRayAngleGaussianNoise(Settings);
}

FAGX_RayAngleGaussianNoiseSettings UAGX_LidarSensorComponent::GetRayAngleNoiseSettings() const
{
	return RayAngleNoiseSettings;
}

UNiagaraComponent* UAGX_LidarSensorComponent::GetSpawnedNiagaraSystemComponent()
{
	return NiagaraSystemComponent;
}

void UAGX_LidarSensorComponent::UpdateNativeTransform()
{
	if (HasNative())
		NativeBarrier.SetTransform(GetComponentTransform());
}

bool UAGX_LidarSensorComponent::AddOutput(FAGX_LidarOutputBase& InOutput)
{
	auto Native = GetOrCreateNative();
	if (Native == nullptr)
		return false;

	auto OutputNative = InOutput.GetOrCreateNative();
	if (OutputNative == nullptr)
		return false;

	Native->AddOutput(*OutputNative);
	return true;
}

bool UAGX_LidarSensorComponent::HasNative() const
{
	return NativeBarrier.HasNative();
}

uint64 UAGX_LidarSensorComponent::GetNativeAddress() const
{
	if (!HasNative())
		return 0;

	NativeBarrier.IncrementRefCount();
	return NativeBarrier.GetNativeAddress();
}

void UAGX_LidarSensorComponent::SetNativeAddress(uint64 NativeAddress)
{
	check(!HasNative());
	NativeBarrier.SetNativeAddress(NativeAddress);
	NativeBarrier.DecrementRefCount();
}

FLidarBarrier* UAGX_LidarSensorComponent::GetOrCreateNative()
{
	if (HasNative())
		return GetNative();

	if (ModelParameters == nullptr)
	{
		FAGX_NotificationUtilities::ShowNotification(
			FString::Printf(
				TEXT(
					"No Model Parameters selected for Lidar Sensor '%s' in '%s'. Make sure a valid "
					"Model Parameter Asset has been selected."),
				*GetName(), *GetLabelSafe(GetOwner())),
			SNotificationItem::CS_Fail);
		return nullptr;
	}

	if (!ModelParameters->IsA(FAGX_SensorUtilities::GetParameterTypeFrom(GetModel())))
	{
		FAGX_NotificationUtilities::ShowNotification(
			FString::Printf(
				TEXT("Lidar Sensor '%s' in '%s': the assigned Model Parameters Asset is not "
					 "compatible with the selected Model. Assign a Model Parameters Asset of the "
					 "appropriate type."),
				*GetName(), *GetLabelSafe(GetOwner())),
			SNotificationItem::CS_Fail);
		return nullptr;
	}

	if (Model == EAGX_LidarModel::CustomRayPattern)
	{
		PatternFetcher.SetLidar(this);
		NativeBarrier.AllocateNativeCustomRayPattern(PatternFetcher);
	}
	else
	{
		NativeBarrier.AllocateNative(Model, *ModelParameters);
	}

	if (!NativeBarrier.HasNative())
	{
		FAGX_NotificationUtilities::ShowNotification(
			FString::Printf(
				TEXT("Lidar Sensor '%s' in '%s': unable to create Native AGX Lidar given the Model "
					 "and ModelParameters."),
				*GetName(), *GetLabelSafe(GetOwner())),
			SNotificationItem::CS_Fail);
		return nullptr;
	}

	UpdateNativeProperties();
	return GetNative();
}

FLidarBarrier* UAGX_LidarSensorComponent::GetNative()
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

const FLidarBarrier* UAGX_LidarSensorComponent::GetNative() const
{
	if (!HasNative())
		return nullptr;

	return &NativeBarrier;
}

void UAGX_LidarSensorComponent::CopyFrom(const UAGX_LidarSensorComponent& Source)
{
	bEnabled = Source.bEnabled;
	Model = Source.Model;
	Range = Source.Range;
	BeamDivergence = Source.BeamDivergence;
	BeamExitRadius = Source.BeamExitRadius;
	ModelParameters = Source.ModelParameters;
	RaytraceDepth = Source.RaytraceDepth;
	bEnableRemovePointsMisses = Source.bEnableRemovePointsMisses;
	bEnableDistanceGaussianNoise = Source.bEnableDistanceGaussianNoise;
	DistanceNoiseSettings = Source.DistanceNoiseSettings;
	bEnableRayAngleGaussianNoise = Source.bEnableRayAngleGaussianNoise;
	RayAngleNoiseSettings = Source.RayAngleNoiseSettings;
}

#if WITH_EDITOR
void UAGX_LidarSensorComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bEnableRendering && NiagaraSystemAsset != nullptr)
	{
		NiagaraSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			NiagaraSystemAsset, this, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator,
			FVector::OneVector, EAttachLocation::Type::KeepRelativeOffset, false,
			ENCPoolMethod::None);
	}
}

void UAGX_LidarSensorComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);

	if (HasNative())
		NativeBarrier.ReleaseNative();
}

void UAGX_LidarSensorComponent::DestroyComponent(bool bPromoteChildren)
{
	if (NiagaraSystemComponent != nullptr)
		NiagaraSystemComponent->DestroyComponent();

	Super::DestroyComponent(bPromoteChildren);
}

bool UAGX_LidarSensorComponent::CanEditChange(const FProperty* InProperty) const
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
			GET_MEMBER_NAME_CHECKED(ThisClass, Model),
			GET_MEMBER_NAME_CHECKED(ThisClass, ModelParameters),
			GET_MEMBER_NAME_CHECKED(ThisClass, bEnableRendering),
			GET_MEMBER_NAME_CHECKED(ThisClass, NiagaraSystemAsset)};

		if (PropertiesNotEditableDuringPlay.Contains(InProperty->GetFName()))
		{
			return false;
		}
	}

	return SuperCanEditChange;
}

TStructOnScope<FActorComponentInstanceData> UAGX_LidarSensorComponent::GetComponentInstanceData()
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

void UAGX_LidarSensorComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that this object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_LidarSensorComponent::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_LidarSensorComponent::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	AGX_COMPONENT_DEFAULT_DISPATCHER(Range);
	AGX_COMPONENT_DEFAULT_DISPATCHER(BeamDivergence);
	AGX_COMPONENT_DEFAULT_DISPATCHER(BeamExitRadius);
	AGX_COMPONENT_DEFAULT_DISPATCHER(RaytraceDepth);

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_LidarSensorComponent, bEnabled),
		[](ThisClass* This) { This->SetEnabled(This->bEnabled); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_LidarSensorComponent, bEnableRemovePointsMisses),
		[](ThisClass* This)
		{ This->SetEnableRemovePointsMisses(This->bEnableRemovePointsMisses); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_LidarSensorComponent, bEnableDistanceGaussianNoise),
		[](ThisClass* This)
		{ This->SetEnableDistanceGaussianNoise(This->bEnableDistanceGaussianNoise); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_LidarSensorComponent, bEnableRayAngleGaussianNoise),
		[](ThisClass* This)
		{ This->SetEnableRayAngleGaussianNoise(This->bEnableRayAngleGaussianNoise); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_LidarSensorComponent, DistanceNoiseSettings),
		[](ThisClass* This) { This->SetDistanceNoiseSettings(This->DistanceNoiseSettings); });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_LidarSensorComponent, RayAngleNoiseSettings),
		[](ThisClass* This) { This->SetRayAngleNoiseSettings(This->RayAngleNoiseSettings); });
}
#endif // WITH_EDITOR

bool UAGX_LidarSensorComponent::IsCustomParametersSupported() const
{
	return Model == EAGX_LidarModel::CustomRayPattern ||
		   Model == EAGX_LidarModel::GenericHorizontalSweep;
}

void UAGX_LidarSensorComponent::UpdateNativeProperties()
{
	AGX_CHECK(HasNative());

	if (IsCustomParametersSupported())
	{
		NativeBarrier.SetRange(Range);
		NativeBarrier.SetBeamDivergence(BeamDivergence);
		NativeBarrier.SetBeamExitRadius(BeamExitRadius);
		SetEnableDistanceGaussianNoise(bEnableDistanceGaussianNoise);
		SetEnableRayAngleGaussianNoise(bEnableRayAngleGaussianNoise);
	}

	NativeBarrier.SetEnableRemoveRayMisses(bEnableRemovePointsMisses);
	SetRaytraceDepth(RaytraceDepth);
	NativeBarrier.SetEnabled(bEnabled);
}

TArray<FTransform> UAGX_LidarSensorComponent::FetchRayTransforms()
{
	AGX_CHECK(Model == EAGX_LidarModel::CustomRayPattern);
	if (!OnFetchRayTransforms.IsBound())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Lidar Sensor '%s' in '%s' uses Custom Scan Pattern but the "
				 "OnFetchRayTransforms delegate has not been assinged. Assign the "
				 "OnFetchRayTransforms delegate in order to use a Custom scan pattern."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return TArray<FTransform>();
	}

	return OnFetchRayTransforms.Execute();
}

FAGX_CustomPatternInterval UAGX_LidarSensorComponent::FetchNextInterval()
{
	AGX_CHECK(Model == EAGX_LidarModel::CustomRayPattern);
	if (!OnFetchNextPatternInterval.IsBound())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Lidar Sensor '%s' in '%s' uses Custom Scan Pattern but the "
				 "FOnFetchNextPatternInterval delegate has not been assinged. Assign the "
				 "FOnFetchNextPatternInterval delegate in order to use a Custom scan pattern."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return FAGX_CustomPatternInterval();
	}

	double TimeStamp = 0.0;
	if (UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(this))
		TimeStamp = Sim->GetTimeStamp();

	return OnFetchNextPatternInterval.Execute(TimeStamp);
}

#undef LOCTEXT_NAMESPACE
