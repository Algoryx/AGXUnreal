// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainWheelSettings.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "Import/AGX_ImportContext.h"
#include "Terrain/TerrainWheelBarrier.h"
#include "Utilities/AGX_ImportRuntimeUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "Engine/World.h"
#include "UObject/UObjectGlobals.h"

namespace AGX_TerrainWheelSettings_helpers
{
	FString CreateSettingsName(const FTerrainWheelBarrier& Barrier, FAGX_ImportContext& Context)
	{
		const FString CleanBarrierName =
			FAGX_ImportRuntimeUtilities::RemoveModelNameFromBarrierName(
				Barrier.GetName(), &Context);
		const FString BaseName = CleanBarrierName.IsEmpty() ? TEXT("TerrainWheelSettings") : CleanBarrierName;
		return FAGX_ObjectUtilities::SanitizeAndMakeNameUnique(
			Context.Outer, FString::Printf(TEXT("AGX_TWS_%s"), *BaseName),
			UAGX_TerrainWheelSettings::StaticClass());
	}
}

void UAGX_TerrainWheelSettings::SetSlipRatioVxAngularEquivalentThreshold(double InThreshold)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		SlipRatioVxAngularEquivalentThreshold, InThreshold,
		SetSlipRatioVxAngularEquivalentThreshold);
}

double UAGX_TerrainWheelSettings::GetSlipRatioVxAngularEquivalentThreshold() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(
		SlipRatioVxAngularEquivalentThreshold, GetSlipRatioVxAngularEquivalentThreshold);
}

void UAGX_TerrainWheelSettings::SetSlipRatioOmegaYThreshold(double InThreshold)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		SlipRatioOmegaYThreshold, InThreshold, SetSlipRatioOmegaYThreshold);
}

double UAGX_TerrainWheelSettings::GetSlipRatioOmegaYThreshold() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(SlipRatioOmegaYThreshold, GetSlipRatioOmegaYThreshold);
}

void UAGX_TerrainWheelSettings::SetSlipRatioSmoothingAngularSpeed(double InSpeed)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		SlipRatioSmoothingAngularSpeed, InSpeed, SetSlipRatioSmoothingAngularSpeed);
}

double UAGX_TerrainWheelSettings::GetSlipRatioSmoothingAngularSpeed() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(
		SlipRatioSmoothingAngularSpeed, GetSlipRatioSmoothingAngularSpeed);
}

void UAGX_TerrainWheelSettings::SetAngularIntegrationStep(double InStep)
{
	AGX_ASSET_SETTER_IMPL_VALUE(AngularIntegrationStep, InStep, SetAngularIntegrationStep);
}

double UAGX_TerrainWheelSettings::GetAngularIntegrationStep() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(AngularIntegrationStep, GetAngularIntegrationStep);
}

void UAGX_TerrainWheelSettings::SetPressureSinkageModel(
	EAGX_TerrainWheelPressureSinkageModel InModel)
{
	AGX_ASSET_SETTER_IMPL_VALUE(PressureSinkageModel, InModel, SetPressureSinkageModel);
}

EAGX_TerrainWheelPressureSinkageModel UAGX_TerrainWheelSettings::GetPressureSinkageModel() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(PressureSinkageModel, GetPressureSinkageModel);
}

void UAGX_TerrainWheelSettings::SetEnableComputeRearAngleFromFrontAngle(bool InEnable)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		bEnableComputeRearAngleFromFrontAngle, InEnable,
		SetEnableComputeRearAngleFromFrontAngle);
}

bool UAGX_TerrainWheelSettings::GetEnableComputeRearAngleFromFrontAngle() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(
		bEnableComputeRearAngleFromFrontAngle, GetEnableComputeRearAngleFromFrontAngle);
}

void UAGX_TerrainWheelSettings::SetEnableComputeMaximumNormalStressAngleFromFrontAngle(
	bool InEnable)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		bEnableComputeMaximumNormalStressAngleFromFrontAngle, InEnable,
		SetEnableComputeMaximumNormalStressAngleFromFrontAngle);
}

bool UAGX_TerrainWheelSettings::GetEnableComputeMaximumNormalStressAngleFromFrontAngle() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(
		bEnableComputeMaximumNormalStressAngleFromFrontAngle,
		GetEnableComputeMaximumNormalStressAngleFromFrontAngle);
}

void UAGX_TerrainWheelSettings::SetEnableAGXDebugRendering(bool InEnable)
{
	AGX_ASSET_SETTER_IMPL_VALUE(bEnableAGXDebugRendering, InEnable, SetEnableAGXDebugRendering);
}

void UAGX_TerrainWheelSettings::CommitToAsset()
{
	if (IsInstance())
	{
		if (FTerrainWheelSettingsBarrier* Barrier = GetNative();
			Barrier != nullptr && Asset.IsValid())
		{
#if WITH_EDITOR
			Asset->Modify();
#endif
			Asset->CopyFrom(*Barrier);

			// AGX has no getter for this setting, so keep the Unreal-side value when committing.
			Asset->bEnableAGXDebugRendering = bEnableAGXDebugRendering;
#if WITH_EDITOR
			FAGX_ObjectUtilities::MarkAssetDirty(*Asset);
#endif
		}
	}
	else if (Instance != nullptr)
	{
		Instance->CommitToAsset();
	}
}

void UAGX_TerrainWheelSettings::CopyFrom(
	const FTerrainWheelBarrier& Source, FAGX_ImportContext* Context)
{
	if (!Source.HasNative())
		return;

	FTerrainWheelSettingsBarrier SettingsBarrier = Source.GetTerrainWheelSettings();
	if (!SettingsBarrier.HasNative())
		return;

	CopyFrom(SettingsBarrier);

	if (Context == nullptr || Context->TerrainWheelSettings == nullptr)
		return; // We are done.

	Rename(*AGX_TerrainWheelSettings_helpers::CreateSettingsName(Source, *Context));
	AGX_CHECK(!Context->TerrainWheelSettings->Contains(ImportGuid));
	Context->TerrainWheelSettings->Add(ImportGuid, this);
}

void UAGX_TerrainWheelSettings::CopyFrom(const FTerrainWheelSettingsBarrier& Source)
{
	if (!Source.HasNative())
		return;

	ImportGuid = Source.GetGuid();
	SlipRatioVxAngularEquivalentThreshold = Source.GetSlipRatioVxAngularEquivalentThreshold();
	SlipRatioOmegaYThreshold = Source.GetSlipRatioOmegaYThreshold();
	SlipRatioSmoothingAngularSpeed = Source.GetSlipRatioSmoothingAngularSpeed();
	AngularIntegrationStep = Source.GetAngularIntegrationStep();
	PressureSinkageModel = Source.GetPressureSinkageModel();
	bEnableComputeRearAngleFromFrontAngle = Source.GetEnableComputeRearAngleFromFrontAngle();
	bEnableComputeMaximumNormalStressAngleFromFrontAngle =
		Source.GetEnableComputeMaximumNormalStressAngleFromFrontAngle();
}

void UAGX_TerrainWheelSettings::CopyFrom(const UAGX_TerrainWheelSettings* Source)
{
	if (Source == nullptr)
		return;

	SlipRatioVxAngularEquivalentThreshold = Source->SlipRatioVxAngularEquivalentThreshold;
	SlipRatioOmegaYThreshold = Source->SlipRatioOmegaYThreshold;
	SlipRatioSmoothingAngularSpeed = Source->SlipRatioSmoothingAngularSpeed;
	AngularIntegrationStep = Source->AngularIntegrationStep;
	PressureSinkageModel = Source->PressureSinkageModel;
	bEnableComputeRearAngleFromFrontAngle = Source->bEnableComputeRearAngleFromFrontAngle;
	bEnableComputeMaximumNormalStressAngleFromFrontAngle =
		Source->bEnableComputeMaximumNormalStressAngleFromFrontAngle;
	bEnableAGXDebugRendering = Source->bEnableAGXDebugRendering;
	ImportGuid = Source->ImportGuid;
}

UAGX_TerrainWheelSettings* UAGX_TerrainWheelSettings::GetInstance()
{
	if (IsInstance())
		return this;

	return Instance.Get();
}

UAGX_TerrainWheelSettings* UAGX_TerrainWheelSettings::GetOrCreateInstance(
	const UWorld* PlayingWorld)
{
	if (IsInstance())
		return this;

	UAGX_TerrainWheelSettings* InstancePtr = Instance.Get();
	if (InstancePtr != nullptr)
		return InstancePtr;

	if (PlayingWorld == nullptr || !PlayingWorld->IsGameWorld())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Could not create runtime instance for Terrain Wheel Settings asset '%s' because "
				 "no game world to create it in was given."),
			*GetPathName());
	}

	const FString InstanceName = GetName() + TEXT("_Instance");
	InstancePtr = NewObject<UAGX_TerrainWheelSettings>(
		GetTransientPackage(), UAGX_TerrainWheelSettings::StaticClass(), *InstanceName,
		RF_Transient);
	InstancePtr->Asset = this;
	InstancePtr->CopyFrom(this);
	InstancePtr->CreateNative();
	Instance = InstancePtr;
	return InstancePtr;
}

UAGX_TerrainWheelSettings* UAGX_TerrainWheelSettings::GetAsset()
{
	if (IsInstance())
		return Asset.Get();

	return this;
}

bool UAGX_TerrainWheelSettings::IsInstance() const
{
	if (GetOuter() == GetTransientPackage() || Cast<UWorld>(GetOuter()) != nullptr)
		return true;

	const bool bIsInstance = Asset != nullptr;
	AGX_CHECK(bIsInstance != IsAsset());
	return bIsInstance;
}

bool UAGX_TerrainWheelSettings::HasNative() const
{
	if (IsInstance())
		return NativeBarrier.HasNative();

	return Instance != nullptr && Instance->HasNative();
}

FTerrainWheelSettingsBarrier* UAGX_TerrainWheelSettings::GetNative()
{
	return const_cast<FTerrainWheelSettingsBarrier*>(
		const_cast<const ThisClass*>(this)->GetNative());
}

const FTerrainWheelSettingsBarrier* UAGX_TerrainWheelSettings::GetNative() const
{
	if (IsInstance())
		return NativeBarrier.HasNative() ? &NativeBarrier : nullptr;

	return Instance != nullptr ? Instance->GetNative() : nullptr;
}

FTerrainWheelSettingsBarrier* UAGX_TerrainWheelSettings::GetOrCreateNative()
{
	if (IsInstance())
	{
		if (!HasNative())
			CreateNative();

		return GetNative();
	}

	if (Instance == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("GetOrCreateNative() called on '%s' but Instance == nullptr. "
				 "Call GetOrCreateInstance() first."),
			*GetName());
		return nullptr;
	}

	return Instance->GetOrCreateNative();
}

void UAGX_TerrainWheelSettings::UpdateNativeProperties()
{
	if (!IsInstance() || !HasNative())
		return;

	NativeBarrier.SetSlipRatioVxAngularEquivalentThreshold(SlipRatioVxAngularEquivalentThreshold);
	NativeBarrier.SetSlipRatioOmegaYThreshold(SlipRatioOmegaYThreshold);
	NativeBarrier.SetSlipRatioSmoothingAngularSpeed(SlipRatioSmoothingAngularSpeed);
	NativeBarrier.SetAngularIntegrationStep(AngularIntegrationStep);
	NativeBarrier.SetPressureSinkageModel(PressureSinkageModel);
	NativeBarrier.SetEnableComputeRearAngleFromFrontAngle(bEnableComputeRearAngleFromFrontAngle);
	NativeBarrier.SetEnableComputeMaximumNormalStressAngleFromFrontAngle(
		bEnableComputeMaximumNormalStressAngleFromFrontAngle);
	NativeBarrier.SetEnableAGXDebugRendering(bEnableAGXDebugRendering);
}

void UAGX_TerrainWheelSettings::PostInitProperties()
{
	Super::PostInitProperties();

#if WITH_EDITOR
	InitPropertyDispatcher();
#endif
}

#if WITH_EDITOR
void UAGX_TerrainWheelSettings::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_TerrainWheelSettings::InitPropertyDispatcher()
{
	auto& PropertyDispatcher = FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
		return;

	AGX_ASSET_DEFAULT_DISPATCHER(SlipRatioVxAngularEquivalentThreshold);
	AGX_ASSET_DEFAULT_DISPATCHER(SlipRatioOmegaYThreshold);
	AGX_ASSET_DEFAULT_DISPATCHER(SlipRatioSmoothingAngularSpeed);
	AGX_ASSET_DEFAULT_DISPATCHER(AngularIntegrationStep);
	AGX_ASSET_DEFAULT_DISPATCHER(PressureSinkageModel);
	AGX_ASSET_DEFAULT_DISPATCHER_BOOL(EnableComputeRearAngleFromFrontAngle);
	AGX_ASSET_DEFAULT_DISPATCHER_BOOL(EnableComputeMaximumNormalStressAngleFromFrontAngle);
	AGX_ASSET_DEFAULT_DISPATCHER_BOOL(EnableAGXDebugRendering);
}
#endif // WITH_EDITOR

void UAGX_TerrainWheelSettings::CreateNative()
{
	if (IsInstance())
	{
		if (HasNative())
			return;

		NativeBarrier.AllocateNative();

		if (!HasNative())
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("UAGX_TerrainWheelSettings '%s' failed to allocate AGX native instance."),
				*GetName());
			return;
		}

		UpdateNativeProperties();
		return;
	}

	if (Instance == nullptr)
	{
		UE_LOG(
			LogAGX, Error, TEXT("CreateNative() called on asset '%s' but Instance == nullptr."),
			*GetName());
		return;
	}

	Instance->CreateNative();
}
