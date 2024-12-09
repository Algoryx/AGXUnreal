// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_LidarAmbientMaterial.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"

// Unreal Engine includes.
#include "Engine/World.h"
#include "UObject/Package.h"

void UAGX_LidarAmbientMaterial::SetRefractiveIndex(float InRefractiveIndex)
{
	AGX_ASSET_SETTER_IMPL_VALUE(RefractiveIndex, InRefractiveIndex, SetRefractiveIndex);
}

float UAGX_LidarAmbientMaterial::GetRefractiveIndex() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(RefractiveIndex, GetRefractiveIndex);
}

void UAGX_LidarAmbientMaterial::SetAttenuationCoefficient(float InAttenuationCoefficient)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		AttenuationCoefficient, InAttenuationCoefficient, SetAttenuationCoefficient);
}

float UAGX_LidarAmbientMaterial::GetAttenuationCoefficient() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(AttenuationCoefficient, GetAttenuationCoefficient);
}

void UAGX_LidarAmbientMaterial::SetReturnProbabilityScaling(
	float InScalingParameter)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		ReturnProbabilityScaling, InScalingParameter,
		SetReturnProbabilityScaling);
}

float UAGX_LidarAmbientMaterial::GetReturnProbabilityScaling() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(
		ReturnProbabilityScaling, GetReturnProbabilityScaling);
}

void UAGX_LidarAmbientMaterial::SetReturnGammaDistributionShapeParameter(float InShapeParameter)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		ReturnGammaDistributionShapeParameter, InShapeParameter,
		SetReturnGammaDistributionShapeParameter);
}

float UAGX_LidarAmbientMaterial::GetReturnGammaDistributionShapeParameter() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(
		ReturnGammaDistributionShapeParameter, GetReturnGammaDistributionShapeParameter);
}

void UAGX_LidarAmbientMaterial::SetReturnGammaDistributionScaleParameter(float InScaleParameter)
{
	AGX_ASSET_SETTER_IMPL_VALUE(
		ReturnGammaDistributionScaleParameter, InScaleParameter,
		SetReturnGammaDistributionScaleParameter);
}

float UAGX_LidarAmbientMaterial::GetReturnGammaDistributionScaleParameter() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(
		ReturnGammaDistributionScaleParameter, GetReturnGammaDistributionScaleParameter);
}

bool UAGX_LidarAmbientMaterial::HasNative() const
{
	if (Instance != nullptr)
	{
		AGX_CHECK(!IsInstance());
		return Instance->HasNative();
	}

	return NativeBarrier.HasNative();
}

FRtAmbientMaterialBarrier* UAGX_LidarAmbientMaterial::GetNative()
{
	if (Instance != nullptr)
	{
		AGX_CHECK(!IsInstance());
		return Instance->GetNative();
	}

	return HasNative() ? &NativeBarrier : nullptr;
}

const FRtAmbientMaterialBarrier* UAGX_LidarAmbientMaterial::GetNative() const
{
	if (Instance != nullptr)
	{
		AGX_CHECK(!IsInstance());
		return Instance->GetNative();
	}

	return HasNative() ? &NativeBarrier : nullptr;
}

void UAGX_LidarAmbientMaterial::ReleaseNative()
{
	if (Instance != nullptr)
	{
		Instance->ReleaseNative();
		return;
	}

	if (HasNative())
	{
		NativeBarrier.ReleaseNative();
	}
}

void UAGX_LidarAmbientMaterial::CommitToAsset()
{
	if (IsInstance())
	{
		if (FRtAmbientMaterialBarrier* Barrier = GetNative())
		{
#if WITH_EDITOR
			Asset->Modify();
#endif
			Asset->CopyFrom(*Barrier);
#if WITH_EDITOR
			FAGX_ObjectUtilities::MarkAssetDirty(*Asset);
#endif
		}
	}
	else if (Instance != nullptr) // IsAsset
	{
		Instance->CommitToAsset();
	}
}

UAGX_LidarAmbientMaterial* UAGX_LidarAmbientMaterial::CreateInstanceFromAsset(
	UWorld* PlayingWorld, UAGX_LidarAmbientMaterial& Source)
{
	check(!Source.IsInstance());
	check(PlayingWorld);
	check(PlayingWorld->IsGameWorld());

	const FString InstanceName = Source.GetName() + "_Instance";

	UAGX_LidarAmbientMaterial* NewInstance = NewObject<UAGX_LidarAmbientMaterial>(
		GetTransientPackage(), UAGX_LidarAmbientMaterial::StaticClass(), *InstanceName,
		RF_Transient);
	NewInstance->Asset = &Source;
	NewInstance->CopyProperties(Source);
	NewInstance->CreateNative();

	return NewInstance;
}

UAGX_LidarAmbientMaterial* UAGX_LidarAmbientMaterial::GetOrCreateInstance(UWorld* PlayingWorld)
{
	if (IsInstance())
	{
		return this;
	}

	UAGX_LidarAmbientMaterial* InstancePtr = Instance.Get();
	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr = CreateInstanceFromAsset(PlayingWorld, *this);
		Instance = InstancePtr;
	}

	return InstancePtr;
}

FRtAmbientMaterialBarrier* UAGX_LidarAmbientMaterial::GetOrCreateNative()
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("GetOrCreateNative was called on UAGX_LidarAmbientMaterial '%s'"
					 "who's instance is nullptr. Ensure e.g. GetOrCreateInstance is called prior "
					 "to calling this function."),
				*GetName());
			return nullptr;
		}

		return Instance->GetOrCreateNative();
	}

	AGX_CHECK(IsInstance());
	if (!HasNative())
	{
		CreateNative();
	}
	return GetNative();
}

void UAGX_LidarAmbientMaterial::UpdateNativeProperties()
{
	if (!HasNative())
		return;

	AGX_CHECK(IsInstance());
	NativeBarrier.SetRefractiveIndex(RefractiveIndex);
	NativeBarrier.SetAttenuationCoefficient(AttenuationCoefficient);
	NativeBarrier.SetReturnProbabilityScaling(
		ReturnProbabilityScaling);
	NativeBarrier.SetReturnGammaDistributionShapeParameter(ReturnGammaDistributionShapeParameter);
	NativeBarrier.SetReturnGammaDistributionScaleParameter(ReturnGammaDistributionScaleParameter);
}

bool UAGX_LidarAmbientMaterial::IsInstance() const
{
	// An instance of this class will always have a reference to it's corresponding Asset.
	// An asset will never have this reference set.
	const bool bIsInstance = Asset != nullptr;
	AGX_CHECK(bIsInstance != IsAsset());
	return bIsInstance;
}

void UAGX_LidarAmbientMaterial::CopyFrom(const FRtAmbientMaterialBarrier& Source)
{
	RefractiveIndex = Source.GetRefractiveIndex();
	AttenuationCoefficient = Source.GetAttenuationCoefficient();
	ReturnProbabilityScaling =
		Source.GetReturnProbabilityScaling();
	ReturnGammaDistributionShapeParameter = Source.GetReturnGammaDistributionShapeParameter();
	ReturnGammaDistributionScaleParameter = Source.GetReturnGammaDistributionScaleParameter();
}

void UAGX_LidarAmbientMaterial::CopyProperties(const UAGX_LidarAmbientMaterial& Source)
{
	RefractiveIndex = Source.GetRefractiveIndex();
	AttenuationCoefficient = Source.GetAttenuationCoefficient();
	ReturnProbabilityScaling =
		Source.GetReturnProbabilityScaling();
	ReturnGammaDistributionShapeParameter = Source.GetReturnGammaDistributionShapeParameter();
	ReturnGammaDistributionScaleParameter = Source.GetReturnGammaDistributionScaleParameter();
}

void UAGX_LidarAmbientMaterial::CreateNative()
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("CreateNative was called on UAGX_LidarAmbientMaterial '%s' who's "
					 "instance is nullptr. "
					 "Ensure e.g. GetOrCreateInstance is called prior to calling this function."),
				*GetName());
			return;
		}
		return Instance->CreateNative();
	}

	AGX_CHECK(IsInstance());
	if (NativeBarrier.HasNative())
	{
		NativeBarrier.ReleaseNative();
	}

	NativeBarrier.AllocateNative();
	check(HasNative());

	UpdateNativeProperties();
}

#if WITH_EDITOR
void UAGX_LidarAmbientMaterial::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_LidarAmbientMaterial::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_LidarAmbientMaterial::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_LidarAmbientMaterial, RefractiveIndex), [](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(RefractiveIndex, SetRefractiveIndex) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_LidarAmbientMaterial, AttenuationCoefficient),
		[](ThisClass* This)
		{ AGX_ASSET_DISPATCHER_LAMBDA_BODY(AttenuationCoefficient, SetAttenuationCoefficient) });

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(
			UAGX_LidarAmbientMaterial, ReturnProbabilityScaling),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				ReturnProbabilityScaling,
				SetReturnProbabilityScaling)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_LidarAmbientMaterial, ReturnGammaDistributionShapeParameter),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				ReturnGammaDistributionShapeParameter, SetReturnGammaDistributionShapeParameter)
		});

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_LidarAmbientMaterial, ReturnGammaDistributionScaleParameter),
		[](ThisClass* This)
		{
			AGX_ASSET_DISPATCHER_LAMBDA_BODY(
				ReturnGammaDistributionScaleParameter, SetReturnGammaDistributionScaleParameter)
		});
}
#endif
