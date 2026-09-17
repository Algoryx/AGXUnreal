// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainWheelDeformationProperties.h"

// AGX Dynamics for Unreal includes.
#include "AGX_AssetGetterSetterImpl.h"
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"

// Unreal Engine includes.
#include "Engine/World.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

UAGX_TerrainWheelDeformationProperties*
UAGX_TerrainWheelDeformationProperties::CreateInstanceFromAsset(
	const UWorld* PlayingWorld, UAGX_TerrainWheelDeformationProperties* Source)
{
	check(Source);
	check(!Source->IsInstance());
	check(PlayingWorld != nullptr);
	check(PlayingWorld->IsGameWorld());

	const FString InstanceName = Source->GetName() + TEXT("_Instance");

	UAGX_TerrainWheelDeformationProperties* NewInstance =
		NewObject<UAGX_TerrainWheelDeformationProperties>(
			GetTransientPackage(), UAGX_TerrainWheelDeformationProperties::StaticClass(),
			*InstanceName, RF_Transient);
	NewInstance->Asset = Source;
	NewInstance->CopyFrom(Source);
	NewInstance->CreateNative();

	return NewInstance;
}

void UAGX_TerrainWheelDeformationProperties::SetEnableTerrainDeformation(bool InEnable)
{
	AGX_ASSET_SETTER_BOOL(EnableTerrainDeformation, InEnable);
}

bool UAGX_TerrainWheelDeformationProperties::GetEnableTerrainDeformation() const
{
	AGX_ASSET_GETTER_BOOL(EnableTerrainDeformation);
}

void UAGX_TerrainWheelDeformationProperties::SetEnableTerrainDisplacement(bool InEnable)
{
	AGX_ASSET_SETTER_BOOL(EnableTerrainDisplacement, InEnable);
}

bool UAGX_TerrainWheelDeformationProperties::GetEnableTerrainDisplacement() const
{
	AGX_ASSET_GETTER_BOOL(EnableTerrainDisplacement);
}

UAGX_TerrainWheelDeformationProperties* UAGX_TerrainWheelDeformationProperties::GetInstance()
{
	if (IsInstance())
		return this;

	return Instance.Get();
}

UAGX_TerrainWheelDeformationProperties*
UAGX_TerrainWheelDeformationProperties::GetOrCreateInstance(const UWorld* PlayingWorld)
{
	if (IsInstance())
		return this;

	UAGX_TerrainWheelDeformationProperties* InstancePtr = Instance.Get();
	if (InstancePtr != nullptr)
		return InstancePtr;

	if (PlayingWorld == nullptr || !PlayingWorld->IsGameWorld())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Could not create runtime instance for Terrain Wheel Deformation Properties "
				 "asset '%s' because no game world to create it in was given."),
			*GetPathName());
		return nullptr;
	}

	InstancePtr = CreateInstanceFromAsset(PlayingWorld, this);
	Instance = InstancePtr;
	return InstancePtr;
}

UAGX_TerrainWheelDeformationProperties* UAGX_TerrainWheelDeformationProperties::GetAsset()
{
	if (IsInstance())
		return Asset.Get();

	return this;
}

bool UAGX_TerrainWheelDeformationProperties::IsInstance() const
{
	if (GetOuter() == GetTransientPackage() || Cast<UWorld>(GetOuter()) != nullptr)
		return true;

	const bool bIsInstance = Asset != nullptr;
	AGX_CHECK(bIsInstance != IsAsset());
	return bIsInstance;
}

bool UAGX_TerrainWheelDeformationProperties::HasNative() const
{
	if (IsInstance())
		return NativeBarrier.HasNative();

	return Instance != nullptr && Instance->HasNative();
}

FTerrainWheelDeformationPropertiesBarrier* UAGX_TerrainWheelDeformationProperties::GetNative()
{
	return const_cast<FTerrainWheelDeformationPropertiesBarrier*>(
		const_cast<const ThisClass*>(this)->GetNative());
}

const FTerrainWheelDeformationPropertiesBarrier*
UAGX_TerrainWheelDeformationProperties::GetNative() const
{
	if (IsInstance())
		return NativeBarrier.HasNative() ? &NativeBarrier : nullptr;

	return Instance != nullptr ? Instance->GetNative() : nullptr;
}

FTerrainWheelDeformationPropertiesBarrier*
UAGX_TerrainWheelDeformationProperties::GetOrCreateNative()
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

void UAGX_TerrainWheelDeformationProperties::UpdateNativeProperties()
{
	if (!IsInstance() || !HasNative())
		return;

	NativeBarrier.SetEnableTerrainDeformation(bEnableTerrainDeformation);
	NativeBarrier.SetEnableTerrainDisplacement(bEnableTerrainDisplacement);
}

void UAGX_TerrainWheelDeformationProperties::PostInitProperties()
{
	Super::PostInitProperties();

#if WITH_EDITOR
	InitPropertyDispatcher();
#endif
}

#if WITH_EDITOR
void UAGX_TerrainWheelDeformationProperties::PostEditChangeChainProperty(
	FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_TerrainWheelDeformationProperties::InitPropertyDispatcher()
{
	auto& PropertyDispatcher = FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
		return;

	AGX_ASSET_DEFAULT_DISPATCHER_BOOL(EnableTerrainDeformation);
	AGX_ASSET_DEFAULT_DISPATCHER_BOOL(EnableTerrainDisplacement);
}
#endif // WITH_EDITOR

void UAGX_TerrainWheelDeformationProperties::CopyFrom(
	const UAGX_TerrainWheelDeformationProperties* Source)
{
	if (Source == nullptr)
		return;

	bEnableTerrainDeformation = Source->bEnableTerrainDeformation;
	bEnableTerrainDisplacement = Source->bEnableTerrainDisplacement;
}

void UAGX_TerrainWheelDeformationProperties::CreateNative()
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
				TEXT("UAGX_TerrainWheelDeformationProperties '%s' failed to allocate AGX native "
					 "instance."),
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
