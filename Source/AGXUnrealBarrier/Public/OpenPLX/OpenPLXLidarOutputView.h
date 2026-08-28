// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

#include "OpenPLXLidarOutputView.generated.h"

struct FOpenPLXLidarOutputViewRef;

struct AGXUNREALBARRIER_API FOpenPLXLidarPointReadFlags
{
	bool bPositions = false;
	bool bIntensities = false;
	bool bTimeStamps = false;
	bool bDistances = false;
	bool bRayPoses = false;
	bool bIsHits = false;
	bool bEntityIds = false;
};

enum class EOpenPLXLidarPackedFieldType : uint8
{
	Float32,
	Float64,
	Int32
};

struct AGXUNREALBARRIER_API FOpenPLXLidarPackedField
{
	FString Name;
	EOpenPLXLidarPackedFieldType Type = EOpenPLXLidarPackedFieldType::Float32;
	int64 Offset = 0;
	int64 Count = 1;
};

/**
 * View into Lidar output data received through OpenPLX.
 *
 * By default this struct references memory owned by the OpenPLX Control Interface. No Lidar output
 * data is copied until data is requested. A newly received view is only valid until another
 * OpenPLX signal receive operation is called since it reuses the underlying Control Interface
 * buffer. Call MakePersistant before storing the view for later use.
 */
USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FOpenPLXLidarOutputView
{
	GENERATED_BODY()

	FOpenPLXLidarOutputView();
	FOpenPLXLidarOutputView(std::shared_ptr<FOpenPLXLidarOutputViewRef> Native);

	bool HasNative() const;

	/// Return the number of points in this Lidar output view.
	int32 GetNumPoints() const;

	/// Returns true if this view contains Lidar point positions.
	bool HasPositions() const;

	/// Returns true if this view contains Lidar point intensities.
	bool HasIntensities() const;

	/// Returns true if this view contains Lidar point timestamps.
	bool HasTimeStamps() const;

	/// Returns true if this view contains Lidar point distances.
	bool HasDistances() const;

	/// Returns true if this view contains Lidar ray poses.
	bool HasRayPoses() const;

	/// Returns true if this view contains Lidar hit flags.
	bool HasIsHits() const;

	/// Returns true if this view contains Lidar entity IDs.
	bool HasEntityIds() const;

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

	/// Read the Lidar point timestamps [s].
	bool ReadTimeStamps(TArray<double>& OutTimeStamps);

	/// Read the Lidar point distances [cm].
	bool ReadDistances(TArray<double>& OutDistances);

	/// Read the Lidar ray poses in Unreal coordinates.
	bool ReadRayPoses(TArray<FTransform>& OutRayPoses);

	/// Read the Lidar hit flags.
	bool ReadIsHits(TArray<bool>& OutIsHits);

	/// Read the Lidar entity IDs.
	bool ReadEntityIds(TArray<int32>& OutEntityIds);

	/**
	 * Read the selected Lidar fields as compact raw point bytes.
	 * No Unreal unit or coordinate conversion is performed.
	 */
	bool ReadRawPointData(
		const FOpenPLXLidarPointReadFlags& ReadFlags, TArray<uint8>& OutData,
		TArray<FOpenPLXLidarPackedField>& OutFields, int64& OutPointStep, bool& bOutAllHits);

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
