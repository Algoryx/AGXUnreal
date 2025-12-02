// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_LidarScanPoint.generated.h"

USTRUCT(BlueprintType)
struct FAGX_LidarScanPoint
{
	GENERATED_BODY()

	FAGX_LidarScanPoint() = default;

	FAGX_LidarScanPoint(
		const FVector& InPosition, double InTimeStamp, double InIntensity, bool InIsValid)
		: Position(InPosition)
		, TimeStamp(InTimeStamp)
		, Intensity(InIntensity)
		, bIsValid(InIsValid)
	{
	}

	explicit FAGX_LidarScanPoint(bool InIsValid)
		: bIsValid(InIsValid)
	{
	}

	/**
	 * The local position of this point [cm].
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Lidar")
	FVector Position {FVector::ZeroVector};

	/**
	 * The moment in game time that this point was measured [s].
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Lidar")
	double TimeStamp {0.0};

	/**
	 * The intensity value asociated with this point, in the range [0..1].
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Lidar")
	float Intensity {0.0};

	/**
	 * Whether this Lidar Scan Point is valid or not.
	 * If this Lidar Scan Point represents a miss, this value is set to false and is set to true
	 * otherwise.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Lidar")
	bool bIsValid {false};
};
