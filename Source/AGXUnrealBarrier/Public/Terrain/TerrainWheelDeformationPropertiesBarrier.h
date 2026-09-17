// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

#include "TerrainWheelDeformationPropertiesBarrier.generated.h"

struct FTerrainWheelDeformationPropertiesRef;

USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FTerrainWheelDeformationPropertiesBarrier
{
	GENERATED_BODY()

public:
	FTerrainWheelDeformationPropertiesBarrier();
	FTerrainWheelDeformationPropertiesBarrier(
		std::shared_ptr<FTerrainWheelDeformationPropertiesRef> Native);

	void SetEnableTerrainDeformation(bool InEnable);
	bool GetEnableTerrainDeformation() const;

	void SetEnableTerrainDisplacement(bool InEnable);
	bool GetEnableTerrainDisplacement() const;

	bool HasNative() const;
	FTerrainWheelDeformationPropertiesRef* GetNative();
	const FTerrainWheelDeformationPropertiesRef* GetNative() const;

	void AllocateNative();
	void ReleaseNative();

private:
	std::shared_ptr<FTerrainWheelDeformationPropertiesRef> NativeRef;
};
