// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "OpenPLX/OpenPLXLidarOutputView.h"

// Unreal Engine includes.
#include "Kismet/BlueprintFunctionLibrary.h"

#include "OpenPLX_LidarOutputViewFunctionLibrary.generated.h"

UCLASS()
class AGXUNREAL_API UOpenPLX_LidarOutputView : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintPure, Category = "OpenPLX Lidar Output View")
	static bool HasNative(const FOpenPLXLidarOutputView& View)
	{
		return View.HasNative();
	}

	/**
	 * Read the Lidar point positions.
	 * The returned positions are in Unreal coordinates and local to the Lidar transform.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX Lidar Output View")
	static bool ReadPositions(
		UPARAM(Ref) FOpenPLXLidarOutputView& View, TArray<FVector>& OutPositions)
	{
		return View.ReadPositions(OutPositions);
	}

	/**
	 * Copy the underlying Lidar output data into memory owned by this view.
	 * A newly received Lidar output view references memory owned by the OpenPLX Control Interface
	 * and is only valid until another read reuses that buffer. Call this before storing the view
	 * for later use. This copies the complete Lidar output buffer.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX Lidar Output View")
	static bool MakePersistant(UPARAM(Ref) FOpenPLXLidarOutputView& View)
	{
		return View.MakePersistant();
	}
};
