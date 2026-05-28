// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Terrain/TerrainWheelSettingsBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Object.h"

#include "AGX_TerrainWheelSettings.generated.h"

/**
 * Contains configuration properties for AGX Terrain Wheel. Several Terrain Wheel Components can
 * share the same Terrain Wheel Settings.
 */
UCLASS(ClassGroup = "AGX_Terrain", Category = "AGX", BlueprintType)
class AGXUNREAL_API UAGX_TerrainWheelSettings : public UObject
{
	GENERATED_BODY()

public:
	UAGX_TerrainWheelSettings() = default;

	/**
	 * Copy property values from the runtime instance to the Terrain Wheel Settings asset the
	 * instance was created from.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Wheel Settings")
	void CommitToAsset();

	UAGX_TerrainWheelSettings* GetInstance();

	/**
	 * If PlayingWorld is an in-game World and this TerrainWheelSettings is an asset, returns a
	 * TerrainWheelSettings instance representing the asset throughout the lifetime of the
	 * GameInstance. If this is already an instance it returns itself.
	 */
	UAGX_TerrainWheelSettings* GetOrCreateInstance(const UWorld* PlayingWorld);

	/**
	 * If this TerrainWheelSettings is an instance, returns the asset it was created from. Else
	 * returns itself.
	 */
	UAGX_TerrainWheelSettings* GetAsset();

	bool IsInstance() const;

	bool HasNative() const;
	FTerrainWheelSettingsBarrier* GetNative();
	const FTerrainWheelSettingsBarrier* GetNative() const;
	FTerrainWheelSettingsBarrier* GetOrCreateNative();

	void UpdateNativeProperties();

private:
	void CreateNative();

private:
	TWeakObjectPtr<UAGX_TerrainWheelSettings> Asset;
	TWeakObjectPtr<UAGX_TerrainWheelSettings> Instance;
	FTerrainWheelSettingsBarrier NativeBarrier;
};
