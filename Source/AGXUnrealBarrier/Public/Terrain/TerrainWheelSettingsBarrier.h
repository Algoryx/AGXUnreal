// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

#include "TerrainWheelSettingsBarrier.generated.h"

struct FTerrainWheelSettingsRef;

/**
 * Acts as an interface to native AGX Terrain Wheel Settings, and encapsulates it so that it is
 * completely hidden from code that includes this file.
 */
USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FTerrainWheelSettingsBarrier
{
	GENERATED_BODY()

public:
	FTerrainWheelSettingsBarrier();
	FTerrainWheelSettingsBarrier(std::shared_ptr<FTerrainWheelSettingsRef> Native);

	bool HasNative() const;
	FTerrainWheelSettingsRef* GetNative();
	const FTerrainWheelSettingsRef* GetNative() const;

	void AllocateNative();
	void ReleaseNative();

private:
	std::shared_ptr<FTerrainWheelSettingsRef> NativeRef;
};
