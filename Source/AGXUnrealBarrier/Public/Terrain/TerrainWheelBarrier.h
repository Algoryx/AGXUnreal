// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

#include "TerrainWheelBarrier.generated.h"

struct FTerrainWheelRef;

USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FTerrainWheelBarrier
{
	GENERATED_BODY()

	FTerrainWheelBarrier();
	FTerrainWheelBarrier(std::shared_ptr<FTerrainWheelRef> Native);

	void AllocateNative(double Radius, double Width);

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
