// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_LidarLambertianOpaqueMaterial.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "Sensors/SensorEnvironmentBarrier.h"

// Unreal Engine includes.
#include "Engine/World.h"
#include "UObject/Package.h"

void UAGX_LidarLambertianOpaqueMaterial::SetReflectivity(float InReflectivity)
{
	AGX_ASSET_SETTER_IMPL_VALUE(Reflectivity, InReflectivity, SetReflectivity);
}

float UAGX_LidarLambertianOpaqueMaterial::GetReflectivity() const
{
	AGX_ASSET_GETTER_IMPL_VALUE(Reflectivity, GetReflectivity);
}

bool UAGX_LidarLambertianOpaqueMaterial::HasNative() const
{
	if (Instance != nullptr)
	{
		AGX_CHECK(!IsInstance());
		return Instance->HasNative();
	}

	return NativeBarrier.HasNative();
}

FRtLambertianOpaqueMaterialBarrier* UAGX_LidarLambertianOpaqueMaterial::GetNative()
{
	if (Instance != nullptr)
	{
		AGX_CHECK(!IsInstance());
		return Instance->GetNative();
	}

	return HasNative() ? &NativeBarrier : nullptr;
}

const FRtLambertianOpaqueMaterialBarrier* UAGX_LidarLambertianOpaqueMaterial::GetNative() const
{
	if (Instance != nullptr)
	{
		AGX_CHECK(!IsInstance());
		return Instance->GetNative();
	}

	return HasNative() ? &NativeBarrier : nullptr;
}

void UAGX_LidarLambertianOpaqueMaterial::ReleaseNative()
{
	if (Instance != nullptr)
	{
		Instance->ReleaseNative();
		return;
	}

	if (HasNative())
	{
		GetNative()->ReleaseNative();
	}
}

void UAGX_LidarLambertianOpaqueMaterial::CommitToAsset()
{
	if (IsInstance())
	{
		if (FRtLambertianOpaqueMaterialBarrier* Barrier = GetNative())
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

UAGX_LidarLambertianOpaqueMaterial* UAGX_LidarLambertianOpaqueMaterial::CreateInstanceFromAsset(
	UWorld* PlayingWorld, UAGX_LidarLambertianOpaqueMaterial& Source)
{
	check(!Source.IsInstance());
	check(PlayingWorld);
	check(PlayingWorld->IsGameWorld());

	const FString InstanceName = Source.GetName() + "_Instance";

	UAGX_LidarLambertianOpaqueMaterial* NewInstance = NewObject<UAGX_LidarLambertianOpaqueMaterial>(
		GetTransientPackage(), UAGX_LidarLambertianOpaqueMaterial::StaticClass(), *InstanceName,
		RF_Transient);
	NewInstance->Asset = &Source;
	NewInstance->CopyProperties(Source);
	NewInstance->CreateNative();

	return NewInstance;
}

UAGX_LidarSurfaceMaterial* UAGX_LidarLambertianOpaqueMaterial::GetOrCreateInstance(
	UWorld* PlayingWorld)
{
	if (IsInstance())
	{
		return this;
	}

	UAGX_LidarLambertianOpaqueMaterial* InstancePtr = Instance.Get();
	if (!InstancePtr && PlayingWorld && PlayingWorld->IsGameWorld())
	{
		InstancePtr = CreateInstanceFromAsset(PlayingWorld, *this);
		Instance = InstancePtr;
	}

	return InstancePtr;
}

FRtLambertianOpaqueMaterialBarrier* UAGX_LidarLambertianOpaqueMaterial::GetOrCreateNative()
{
	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("GetOrCreateNative was called on UAGX_LidarLambertianOpaqueMaterial '%s'"
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

void UAGX_LidarLambertianOpaqueMaterial::UpdateNativeProperties()
{
	if (!HasNative())
		return;

	AGX_CHECK(IsInstance());
	NativeBarrier.SetReflectivity(Reflectivity);
}

bool UAGX_LidarLambertianOpaqueMaterial::IsInstance() const
{
	// An instance of this class will always have a reference to it's corresponding Asset.
	// An asset will never have this reference set.
	const bool bIsInstance = Asset != nullptr;
	AGX_CHECK(bIsInstance != IsAsset());
	return bIsInstance;
}

void UAGX_LidarLambertianOpaqueMaterial::CopyFrom(const FRtLambertianOpaqueMaterialBarrier& Source)
{
	Reflectivity = Source.GetReflectivity();
}

void UAGX_LidarLambertianOpaqueMaterial::CopyProperties(
	const UAGX_LidarLambertianOpaqueMaterial& Source)
{
	Reflectivity = Source.GetReflectivity();
}

void UAGX_LidarLambertianOpaqueMaterial::CreateNative()
{
	if (!FSensorEnvironmentBarrier::IsRaytraceSupported())
		return;

	if (!IsInstance())
	{
		if (Instance == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("CreateNative was called on a UAGX_LidarLambertianOpaqueMaterial who's "
					 "instance is nullptr. "
					 "Ensure e.g. GetOrCreateInstance is called prior to calling this function."));
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
void UAGX_LidarLambertianOpaqueMaterial::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}

void UAGX_LidarLambertianOpaqueMaterial::PostEditChangeChainProperty(
	FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_LidarLambertianOpaqueMaterial::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_LidarLambertianOpaqueMaterial, Reflectivity),
		[](ThisClass* This) { AGX_ASSET_DISPATCHER_LAMBDA_BODY(Reflectivity, SetReflectivity) });
}
#endif
