// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainWheelSettings.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

void UAGX_TerrainWheelSettings::CommitToAsset()
{
	if (IsInstance() && Asset.IsValid())
	{
		UEngine::CopyPropertiesForUnrelatedObjects(this, Asset.Get());
		if (UPackage* Package = Asset->GetPackage())
		{
			Package->SetDirtyFlag(true);
			Package->PackageMarkedDirtyEvent.Broadcast(Package, true);
		}
	}
	else if (Instance.IsValid())
	{
		Instance->CommitToAsset();
	}
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
		RF_Transient, this);
	InstancePtr->Asset = this;
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
}

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
