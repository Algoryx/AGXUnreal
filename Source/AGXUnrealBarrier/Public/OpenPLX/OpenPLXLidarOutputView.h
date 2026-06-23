// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

#include "OpenPLXLidarOutputView.generated.h"

struct FOpenPLXLidarOutputViewRef;

/**
 * View into Lidar output data received through OpenPLX.
 *
 * By default this struct references memory owned by the OpenPLX Control Interface. No Lidar output
 * data is copied until data is requested. A newly received view is only valid until another
 * read operation reuses the underlying Control Interface buffer. Call MakePersistant before storing
 * the view for later use.
 */
USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FOpenPLXLidarOutputView
{
	GENERATED_BODY()

	FOpenPLXLidarOutputView();
	FOpenPLXLidarOutputView(std::shared_ptr<FOpenPLXLidarOutputViewRef> Native);

	bool HasNative() const;

	/// Returns true if this view contains Lidar point positions.
	bool HasPositions() const;

	/// Returns true if this view contains Lidar point intensities.
	bool HasIntensities() const;

	/**
	 * Read the Lidar point positions.
	 * The returned positions are in Unreal coordinates and local to the Lidar transform.
	 */
	bool ReadPositions(TArray<FVector>& OutPositions);

	/**
	 * Read the Lidar point positions transformed by RelativeTo.
	 * The returned positions are in Unreal coordinates.
	 */
	bool ReadPositionsTransformed(const FTransform& RelativeTo, TArray<FVector>& OutPositions);

	/// Read the Lidar point intensities.
	bool ReadIntensities(TArray<float>& OutIntensities);

	/**
	 * Copy the underlying Lidar output data into memory owned by this view.
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
