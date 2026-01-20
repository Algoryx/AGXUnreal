// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

#include "TerrainWheelBarrier.generated.h"

struct FCylinderShapeBarrier;
struct FRigidBodyBarrier;
struct FTerrainWheelRef;
struct FTerrainWheelMaterialBarrier;

USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FTerrainWheelBarrier
{
	GENERATED_BODY()

	FTerrainWheelBarrier();
	FTerrainWheelBarrier(std::shared_ptr<FTerrainWheelRef> Native);

	void SetTerrainWheelMaterial(FTerrainWheelMaterialBarrier& Material);
	FTerrainWheelMaterialBarrier GetTerrainWheelMaterial() const;
	void SetTerrainWheelMaterialToDefault();

	void SetEnableTerrainDeformation(bool InEnable);
	bool GetEnableTerrainDeformation() const;

	void SetEnableTerrainDisplacement(bool InEnable);
	bool GetEnableTerrainDisplacement() const;

	void SetEnableAGXDebugRendering(bool InEnable); // No getter in AGX.

	void AllocateNative(FRigidBodyBarrier& Body, FCylinderShapeBarrier& Cylinder);

	FGuid GetGuid() const;

	bool HasNative() const;
	FTerrainWheelRef* GetNative();
	const FTerrainWheelRef* GetNative() const;
	uint64 GetNativeAddress() const;
	void SetNativeAddress(uint64 Address);
	void ReleaseNative();

	void IncrementRefCount() const;
	void DecrementRefCount() const;

private:
	std::shared_ptr<FTerrainWheelRef> NativeRef;
};
