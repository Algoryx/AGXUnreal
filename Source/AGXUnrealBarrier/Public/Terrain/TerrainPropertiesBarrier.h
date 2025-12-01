// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

#include "TerrainPropertiesBarrier.generated.h"

struct FTerrainPropertiesRef;

USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FTerrainPropertiesBarrier
{
	GENERATED_BODY()

	FTerrainPropertiesBarrier();
	FTerrainPropertiesBarrier(std::shared_ptr<FTerrainPropertiesRef> Native);

	bool HasNative() const;
	void AllocateNative();
	FTerrainPropertiesRef* GetNative();
	const FTerrainPropertiesRef* GetNative() const;

	void ReleaseNative();

private:
	std::shared_ptr<FTerrainPropertiesRef> NativeRef;
};
