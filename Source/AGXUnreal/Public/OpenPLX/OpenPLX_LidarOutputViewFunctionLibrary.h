// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "OpenPLX/OpenPLXLidarOutputView.h"

// Unreal Engine includes.
#include "Kismet/BlueprintFunctionLibrary.h"

#include "OpenPLX_LidarOutputViewFunctionLibrary.generated.h"

class UAGX_LidarSensorComponent;

UCLASS()
class AGXUNREAL_API UOpenPLX_LidarOutputView : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintPure, Category = "OpenPLX Lidar Output View")
	static bool HasNative(const FOpenPLXLidarOutputView& View)
	{
		return View.HasNative();
	}

	UFUNCTION(BlueprintPure, Category = "OpenPLX Lidar Output View")
	static bool HasPositions(const FOpenPLXLidarOutputView& View)
	{
		return View.HasPositions();
	}

	UFUNCTION(BlueprintPure, Category = "OpenPLX Lidar Output View")
	static bool HasIntensities(const FOpenPLXLidarOutputView& View)
	{
		return View.HasIntensities();
	}

	UFUNCTION(BlueprintPure, Category = "OpenPLX Lidar Output View")
	static bool HasTimeStamps(const FOpenPLXLidarOutputView& View)
	{
		return View.HasTimeStamps();
	}

	UFUNCTION(BlueprintPure, Category = "OpenPLX Lidar Output View")
	static bool HasDistances(const FOpenPLXLidarOutputView& View)
	{
		return View.HasDistances();
	}

	UFUNCTION(BlueprintPure, Category = "OpenPLX Lidar Output View")
	static bool HasRayPoses(const FOpenPLXLidarOutputView& View)
	{
		return View.HasRayPoses();
	}

	UFUNCTION(BlueprintPure, Category = "OpenPLX Lidar Output View")
	static bool HasIsHits(const FOpenPLXLidarOutputView& View)
	{
		return View.HasIsHits();
	}

	UFUNCTION(BlueprintPure, Category = "OpenPLX Lidar Output View")
	static bool HasEntityIds(const FOpenPLXLidarOutputView& View)
	{
		return View.HasEntityIds();
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
	 * Read the Lidar point positions transformed by RelativeTo.
	 * The returned positions are in Unreal coordinates.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX Lidar Output View")
	static bool ReadPositionsTransformed(
		UPARAM(Ref) FOpenPLXLidarOutputView& View, const FTransform& RelativeTo,
		TArray<FVector>& OutPositions)
	{
		return View.ReadPositionsTransformed(RelativeTo, OutPositions);
	}

	UFUNCTION(BlueprintCallable, Category = "OpenPLX Lidar Output View")
	static bool ReadIntensities(
		UPARAM(Ref) FOpenPLXLidarOutputView& View, TArray<float>& OutIntensities)
	{
		return View.ReadIntensities(OutIntensities);
	}

	UFUNCTION(BlueprintCallable, Category = "OpenPLX Lidar Output View")
	static bool ReadTimeStamps(
		UPARAM(Ref) FOpenPLXLidarOutputView& View, TArray<double>& OutTimeStamps)
	{
		return View.ReadTimeStamps(OutTimeStamps);
	}

	UFUNCTION(BlueprintCallable, Category = "OpenPLX Lidar Output View")
	static bool ReadDistances(
		UPARAM(Ref) FOpenPLXLidarOutputView& View, TArray<double>& OutDistances)
	{
		return View.ReadDistances(OutDistances);
	}

	UFUNCTION(BlueprintCallable, Category = "OpenPLX Lidar Output View")
	static bool ReadRayPoses(
		UPARAM(Ref) FOpenPLXLidarOutputView& View, TArray<FTransform>& OutRayPoses)
	{
		return View.ReadRayPoses(OutRayPoses);
	}

	UFUNCTION(BlueprintCallable, Category = "OpenPLX Lidar Output View")
	static bool ReadIsHits(
		UPARAM(Ref) FOpenPLXLidarOutputView& View, TArray<bool>& OutIsHits)
	{
		return View.ReadIsHits(OutIsHits);
	}

	UFUNCTION(BlueprintCallable, Category = "OpenPLX Lidar Output View")
	static bool ReadEntityIds(
		UPARAM(Ref) FOpenPLXLidarOutputView& View, TArray<int32>& OutEntityIds)
	{
		return View.ReadEntityIds(OutEntityIds);
	}

	/**
	 * Render the data in this Lidar output view.
	 * If intensities are available then points are colored like AGX Lidar position-intensity
	 * outputs. Otherwise this renders positions only, matching AGX Lidar position outputs.
	 *
	 * LifeTime is how long each point is visible before disappearing [s].
	 * ZeroDistanceSize is the minimum apparent size of a point [cm].
	 * Intensity Scale Factor is a (non-physical) scaling factor that is multiplied with all
	 * intensity values before calculating a color for the corresponding points. I.e. it changes the
	 * sensitivity of the intensity coloration (blue to red).
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX Lidar Output View")
	static bool Render(
		UPARAM(Ref) FOpenPLXLidarOutputView& View, UAGX_LidarSensorComponent* Lidar,
		float LifeTime = 0.12f, float ZeroDistanceSize = 4.f,
		float IntensityScaleFactor = 10.f);

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
