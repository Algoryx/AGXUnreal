// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_LidarSensorComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_Simulation.h"
#include "Import/AGX_ImportContext.h"
#include "Import/AGX_ImportSettings.h"
#include "Sensors/AGX_CustomRayPatternParameters.h"
#include "Sensors/AGX_GenericHorizontalSweepParameters.h"
#include "Sensors/AGX_LidarOutputBase.h"
#include "Sensors/AGX_OusterOS0Parameters.h"
#include "Sensors/AGX_OusterOS1Parameters.h"
#include "Sensors/AGX_OusterOS2Parameters.h"
#include "Sensors/AGX_SensorEnvironmentSubsystem.h"
#include "Sensors/SensorEnvironmentBarrier.h"
#include "Utilities/AGX_ImportRuntimeUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"
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
	static const TCHAR* DefaultNiagaraSystem =
		TEXT("NiagaraSystem'/AGXUnreal/Sensor/Lidar/NS_LidarNiagaraSystem.NS_LidarNiagaraSystem'");
	NiagaraSystemAsset =
		FAGX_ObjectUtilities::GetAssetFromPath<UNiagaraSystem>(DefaultNiagaraSystem);

	NativeBarrier.Reset(new FLidarBarrier());
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

void UAGX_LidarSensorComponent::SetRange(FAGX_RealInterval InRange)
{
	Range = InRange;

	if (HasNative())
		GetNativeAsLidar()->SetRange(InRange);
}

FAGX_RealInterval UAGX_LidarSensorComponent::GetRange() const
{
	if (HasNative())
		return GetNativeAsLidar()->GetRange();

	return Range;
}

void UAGX_LidarSensorComponent::SetBeamDivergence(double InBeamDivergence)
{
	BeamDivergence = InBeamDivergence;

	if (HasNative())
		GetNativeAsLidar()->SetBeamDivergence(InBeamDivergence);
}

double UAGX_LidarSensorComponent::GetBeamDivergence() const
{
	if (HasNative())
		return GetNativeAsLidar()->GetBeamDivergence();

	return BeamDivergence;
}

void UAGX_LidarSensorComponent::SetBeamExitRadius(double InBeamExitRadius)
{
	BeamExitRadius = InBeamExitRadius;

	if (HasNative())
		GetNativeAsLidar()->SetBeamExitRadius(InBeamExitRadius);
}

double UAGX_LidarSensorComponent::GetBeamExitRadius() const
{
	if (HasNative())
		return GetNativeAsLidar()->GetBeamExitRadius();

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
		GetNativeAsLidar()->SetRaytraceDepth(static_cast<size_t>(Depth));
}

int32 UAGX_LidarSensorComponent::GetRaytraceDepth() const
{
	if (HasNative())
	{
		return static_cast<int32>(std::min(
			GetNativeAsLidar()->GetRaytraceDepth(),
			static_cast<size_t>(std::numeric_limits<int32>::max())));
	}

	return RaytraceDepth;
}

void UAGX_LidarSensorComponent::SetEnableRemovePointsMisses(bool bEnable)
{
	bEnableRemovePointsMisses = bEnable;

	if (HasNative())
		GetNativeAsLidar()->SetEnableRemoveRayMisses(bEnable);
}

bool UAGX_LidarSensorComponent::GetEnableRemovePointsMisses() const
{
	if (HasNative())
		return GetNativeAsLidar()->GetEnableRemoveRayMisses();

	return bEnableRemovePointsMisses;
}

void UAGX_LidarSensorComponent::SetEnableDistanceGaussianNoise(bool bEnable)
{
	bEnableDistanceGaussianNoise = bEnable;

	if (HasNative())
	{
		if (bEnable)
		{
			GetNativeAsLidar()->EnableOrUpdateDistanceGaussianNoise(DistanceNoiseSettings);
		}
		else
		{
			GetNativeAsLidar()->DisableDistanceGaussianNoise();
		}
	}
}

bool UAGX_LidarSensorComponent::GetEnableDistanceGaussianNoise() const
{
	if (HasNative())
		return GetNativeAsLidar()->GetEnableDistanceGaussianNoise();

	return bEnableDistanceGaussianNoise;
}

void UAGX_LidarSensorComponent::SetDistanceNoiseSettings(
	FAGX_DistanceGaussianNoiseSettings Settings)
{
	DistanceNoiseSettings = Settings;

	if (HasNative() && GetNativeAsLidar()->GetEnableDistanceGaussianNoise())
		GetNativeAsLidar()->EnableOrUpdateDistanceGaussianNoise(Settings);
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
			GetNativeAsLidar()->EnableOrUpdateRayAngleGaussianNoise(RayAngleNoiseSettings);
		}
		else
		{
			GetNativeAsLidar()->DisableRayAngleGaussianNoise();
		}
	}
}

bool UAGX_LidarSensorComponent::GetEnableRayAngleGaussianNoise() const
{
	if (HasNative())
		return GetNativeAsLidar()->GetEnableRayAngleGaussianNoise();

	return bEnableRayAngleGaussianNoise;
}

void UAGX_LidarSensorComponent::SetRayAngleNoiseSettings(
	FAGX_RayAngleGaussianNoiseSettings Settings)
{
	RayAngleNoiseSettings = Settings;

	if (HasNative() && GetNativeAsLidar()->GetEnableRayAngleGaussianNoise())
		GetNativeAsLidar()->EnableOrUpdateRayAngleGaussianNoise(Settings);
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
	// The Native AGX Lidars Frame is always owned by the Lidar itself and root to the World.
	// Therefore we can use the LocalTransform setter below.
	if (HasNative())
		GetNativeAsLidar()->SetLocalTransform(GetComponentTransform());
}

bool UAGX_LidarSensorComponent::AddOutput(FAGX_LidarOutputBase& InOutput)
{
	if (bOpenPLXImported)
	{
		FAGX_NotificationUtilities::ShowNotification(
			FString::Printf(
				TEXT("Outputs cannot be manually added to Lidar Sensor '%s' in '%s' because it "
					 "was imported from an OpenPLX file which define its outputs. OpenPLX outputs "
					 "are added automatically at BeginPlay for this Lidar Sensor."),
				*GetName(), *GetLabelSafe(GetOwner())),
			SNotificationItem::CS_Fail);
		return false;
	}

	auto Native = static_cast<FLidarBarrier*>(GetOrCreateNative());
	if (Native == nullptr)
		return false;

	auto OutputNative = InOutput.GetOrCreateNative();
	if (OutputNative == nullptr)
		return false;

	Native->AddOutput(*OutputNative);
	return true;
}

FSensorBarrier* UAGX_LidarSensorComponent::CreateNativeImpl()
{
	Super::CreateNativeImpl();

	const bool RaytraceRTXSupported = FSensorEnvironmentBarrier::IsRaytraceSupported();
	if (!RaytraceRTXSupported)
	{
		const FString Message =
			"UAGX_LidarSensorComponent::CreateNativeImpl called, but Lidar raytracing (RTX) is "
			"not supported on this computer, the Lidar Sensor will not "
			"work. To enable Lidar raytracing (RTX) support, use an RTX "
			"Graphical Processing Unit (GPU) with updated driver.";
		UE_LOG(LogAGX, Warning, TEXT("%s"), *Message);
		return nullptr;
	}

	if (Model == EAGX_LidarModel::Invalid)
	{
		FAGX_NotificationUtilities::ShowNotification(
			FString::Printf(
				TEXT("Invalid Model selected for Lidar Sensor '%s' in '%s'. Make sure a valid "
					 "Model has been selected."),
				*GetName(), *GetLabelSafe(GetOwner())),
			SNotificationItem::CS_Fail);
		return nullptr;
	}

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

	auto LidarBarrier = static_cast<FLidarBarrier*>(NativeBarrier.Get());
	if (Model == EAGX_LidarModel::CustomRayPattern)
	{
		PatternFetcher.SetLidar(this);
		LidarBarrier->AllocateNativeCustomRayPattern(PatternFetcher);
	}
	else
	{
		LidarBarrier->AllocateNative(Model, *ModelParameters);
	}

	if (!HasNative())
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
	return LidarBarrier;
}

void UAGX_LidarSensorComponent::CopyFrom(const UAGX_LidarSensorComponent& Source)
{
	bEnabled = Source.bEnabled;
	Model = Source.Model;
	Range = Source.Range;
	BeamDivergence = Source.BeamDivergence;
	BeamExitRadius = Source.BeamExitRadius;
	StepStride = Source.StepStride;
	ModelParameters = Source.ModelParameters;
	RaytraceDepth = Source.RaytraceDepth;
	bEnableRemovePointsMisses = Source.bEnableRemovePointsMisses;
	bEnableDistanceGaussianNoise = Source.bEnableDistanceGaussianNoise;
	DistanceNoiseSettings = Source.DistanceNoiseSettings;
	bEnableRayAngleGaussianNoise = Source.bEnableRayAngleGaussianNoise;
	RayAngleNoiseSettings = Source.RayAngleNoiseSettings;
}

namespace AGX_LidarSensorComponent_helpers
{
	bool ReadModelParameters(
		const FLidarBarrier& Barrier, UAGX_LidarModelParameters& ModelParameters)
	{
		if (auto Parameters = Cast<UAGX_OusterOS0Parameters>(&ModelParameters))
			return Barrier.ReadModelParameters(*Parameters);

		if (auto Parameters = Cast<UAGX_OusterOS1Parameters>(&ModelParameters))
			return Barrier.ReadModelParameters(*Parameters);

		if (auto Parameters = Cast<UAGX_OusterOS2Parameters>(&ModelParameters))
			return Barrier.ReadModelParameters(*Parameters);

		if (auto Parameters = Cast<UAGX_GenericHorizontalSweepParameters>(&ModelParameters))
			return Barrier.ReadModelParameters(*Parameters);

		if (auto Parameters = Cast<UAGX_CustomRayPatternParameters>(&ModelParameters))
			return Barrier.ReadModelParameters(*Parameters);

		return false;
	}

	FString CreateModelParametersName(
		const UAGX_LidarSensorComponent& Lidar, const FLidarBarrier& Barrier,
		FAGX_ImportContext& Context, UClass& ModelParametersType, UObject& Outer)
	{
		const FString CleanBarrierName =
			FAGX_ImportRuntimeUtilities::RemoveModelNameFromBarrierName(
				Lidar, Barrier.GetName(), &Context);
		const FString BaseName =
			CleanBarrierName.IsEmpty() ? ModelParametersType.GetName() : CleanBarrierName;
		return FAGX_ObjectUtilities::SanitizeAndMakeNameUnique(
			&Outer, FString::Printf(TEXT("LMP_%s"), *BaseName),
			UAGX_LidarModelParameters::StaticClass());
	}

	UAGX_LidarModelParameters* CreateModelParameters(
		const UAGX_LidarSensorComponent& Lidar, const FLidarBarrier& Barrier,
		FAGX_ImportContext& Context, EAGX_LidarModel Model)
	{
		UClass* ModelParametersType = FAGX_SensorUtilities::GetParameterTypeFrom(Model);
		if (ModelParametersType == nullptr)
			return nullptr;

		const FGuid Guid = Barrier.GetGuid();
		AGX_CHECK(Context.LidarModelParameters != nullptr);
		AGX_CHECK(!Context.LidarModelParameters->Contains(Guid));

		UObject* Outer = Context.Outer != nullptr ? Context.Outer : GetTransientPackage();
		const FString Name =
			CreateModelParametersName(Lidar, Barrier, Context, *ModelParametersType, *Outer);
		auto Parameters = NewObject<UAGX_LidarModelParameters>(
			Outer, ModelParametersType, FName(*Name), RF_Public | RF_Standalone);
		if (Parameters == nullptr)
			return nullptr;

		FAGX_ImportRuntimeUtilities::OnAssetTypeCreated(*Parameters, Context.SessionGuid);
		Parameters->ImportGuid = Guid;

		if (!ReadModelParameters(Barrier, *Parameters))
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Unable to read Lidar Model Parameters for imported Lidar Sensor '%s'. The "
					 "parameters of Model Parameters asset '%s' may be incorrect."),
				*Barrier.GetName(), *Parameters->GetName());
		}

		Context.LidarModelParameters->Add(Guid, Parameters);
		return Parameters;
	}
}

void UAGX_LidarSensorComponent::CopyFrom(const FSensorBarrier& Barrier, FAGX_ImportContext* Context)
{
	Super::CopyFrom(Barrier, Context);

	const FLidarBarrier& LidarBarrier = static_cast<const FLidarBarrier&>(Barrier);

	bEnabled = LidarBarrier.GetEnabled();

	EAGX_LidarModel ImportedModel = LidarBarrier.GetModel();
	if (ImportedModel == EAGX_LidarModel::Invalid)
	{
		// Lidar Model unknown, but we can match against GenericHorizontalSweep if the
		// ray pattern generator is the Horizontal Sweep Pattern.
		if (LidarBarrier.UsesHorizontalSweepRayPattern())
			ImportedModel = EAGX_LidarModel::GenericHorizontalSweep;
	}

	if (ImportedModel == EAGX_LidarModel::Invalid)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Unable to determine Lidar Model when importing Lidar Sensor Component '%s'. "
				 "Keeping default Model."),
			*GetName());
	}
	else
	{
		Model = ImportedModel;
	}

	Range = LidarBarrier.GetRange();
	BeamDivergence = LidarBarrier.GetBeamDivergence();
	BeamExitRadius = LidarBarrier.GetBeamExitRadius();
	bEnableDistanceGaussianNoise = LidarBarrier.GetEnableDistanceGaussianNoise();
	DistanceNoiseSettings = LidarBarrier.GetDistanceGaussianNoiseSettings();
	bEnableRayAngleGaussianNoise = LidarBarrier.GetEnableRayAngleGaussianNoise();
	RayAngleNoiseSettings = LidarBarrier.GetRayAngleGaussianNoiseSettings();

	bEnableRemovePointsMisses = LidarBarrier.GetEnableRemoveRayMisses();
	RaytraceDepth = static_cast<int32>(std::min(
		LidarBarrier.GetRaytraceDepth(), static_cast<size_t>(std::numeric_limits<int32>::max())));

	if (Context == nullptr || Context->Sensors == nullptr ||
		Context->LidarModelParameters == nullptr)
		return; // We are done.

	ModelParameters = AGX_LidarSensorComponent_helpers::CreateModelParameters(
		*this, LidarBarrier, *Context, ImportedModel);

	SetRelativeTransform(LidarBarrier.GetLocalTransform());

	AGX_CHECK(!Context->Sensors->Contains(ImportGuid));
	Context->Sensors->Add(ImportGuid, this);
}

void UAGX_LidarSensorComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GIsReconstructingBlueprintInstances)
		return;

	if (!HasNative())
		CreateNativeImpl();

	if (HasNative())
	{
		if (auto Se = UAGX_SensorEnvironmentSubsystem::GetFrom(this))
		{
			Se->AddLidar(this);
		}
	}

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
	if (!GIsReconstructingBlueprintInstances && HasNative() &&
		Reason != EEndPlayReason::EndPlayInEditor && Reason != EEndPlayReason::Quit &&
		Reason != EEndPlayReason::LevelTransition)
	{
		if (auto Se = UAGX_SensorEnvironmentSubsystem::GetFrom(this))
		{
			Se->RemoveLidar(this);
		}
	}

	Super::EndPlay(Reason);
}

void UAGX_LidarSensorComponent::DestroyComponent(bool bPromoteChildren)
{
	if (NiagaraSystemComponent != nullptr)
		NiagaraSystemComponent->DestroyComponent();

	Super::DestroyComponent(bPromoteChildren);
}

#if WITH_EDITOR
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
			AGX_MEMBER_NAME(Model), AGX_MEMBER_NAME(ModelParameters),
			AGX_MEMBER_NAME(bEnableRendering), AGX_MEMBER_NAME(NiagaraSystemAsset)};

		if (PropertiesNotEditableDuringPlay.Contains(InProperty->GetFName()))
		{
			return false;
		}
	}

	return SuperCanEditChange;
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
	AGX_COMPONENT_DEFAULT_DISPATCHER_BOOL(EnableRemovePointsMisses);
	AGX_COMPONENT_DEFAULT_DISPATCHER_BOOL(EnableDistanceGaussianNoise);
	AGX_COMPONENT_DEFAULT_DISPATCHER_BOOL(EnableRayAngleGaussianNoise);
	AGX_COMPONENT_DEFAULT_DISPATCHER(DistanceNoiseSettings);
	AGX_COMPONENT_DEFAULT_DISPATCHER(RayAngleNoiseSettings);
}
#endif // WITH_EDITOR

void UAGX_LidarSensorComponent::MarkOutputAsRead()
{
	if (HasNative())
		GetNativeAsLidar()->MarkOutputAsRead();
}

bool UAGX_LidarSensorComponent::IsCustomParametersSupported() const
{
	return Model == EAGX_LidarModel::CustomRayPattern ||
		   Model == EAGX_LidarModel::GenericHorizontalSweep;
}

void UAGX_LidarSensorComponent::UpdateNativeProperties()
{
	AGX_CHECK(HasNative());

	Super::UpdateNativeProperties();
	if (IsCustomParametersSupported())
	{
		GetNativeAsLidar()->SetRange(Range);
		GetNativeAsLidar()->SetBeamDivergence(BeamDivergence);
		GetNativeAsLidar()->SetBeamExitRadius(BeamExitRadius);
		SetEnableDistanceGaussianNoise(bEnableDistanceGaussianNoise);
		SetEnableRayAngleGaussianNoise(bEnableRayAngleGaussianNoise);
	}

	GetNativeAsLidar()->SetEnableRemoveRayMisses(bEnableRemovePointsMisses);
	SetRaytraceDepth(RaytraceDepth);
}

FLidarBarrier* UAGX_LidarSensorComponent::GetNativeAsLidar()
{
	if (!HasNative())
		return nullptr;

	return static_cast<FLidarBarrier*>(NativeBarrier.Get());
}

const FLidarBarrier* UAGX_LidarSensorComponent::GetNativeAsLidar() const
{
	if (!HasNative())
		return nullptr;

	return static_cast<const FLidarBarrier*>(NativeBarrier.Get());
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
