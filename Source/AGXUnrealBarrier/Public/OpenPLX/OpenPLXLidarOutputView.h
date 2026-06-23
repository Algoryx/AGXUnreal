// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

#include "OpenPLXLidarOutputView.generated.h"

struct FOpenPLXLidarOutputViewRef;

USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FOpenPLXLidarOutputView
{
	GENERATED_BODY()

	FOpenPLXLidarOutputView();
	FOpenPLXLidarOutputView(std::shared_ptr<FOpenPLXLidarOutputViewRef> Native);

	bool HasNative() const;
	bool ReadPositions(TArray<FVector>& OutPositions);

	/**
	 * Copy the underlying Lidar output data into memory owned by this view.
	 *
	 * A newly received Lidar output view references memory owned by the OpenPLX Control Interface
	 * and is only valid until another read reuses that buffer. Call this before storing the view
	 * for later use. This copies the complete Lidar output buffer.
	 */
	bool MakePersistant();

	FOpenPLXLidarOutputViewRef* GetNative();
	const FOpenPLXLidarOutputViewRef* GetNative() const;

private:
	std::shared_ptr<FOpenPLXLidarOutputViewRef> NativeRef;
};
