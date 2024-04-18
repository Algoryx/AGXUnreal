// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_DistanceGaussianNoiseSettings.generated.h"

USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_DistanceGaussianNoiseSettings
{
	GENERATED_BODY()

	/**
	 * Mean of the distance gaussian noise [cm].
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar",
		Meta = (ClampMin = "0.0", EditCondition = "bEnableDistanceGaussianNoise"))
	FAGX_Real Mean {0.0};

	/**
	 * Standard deviation of the distance gaussian noise [cm].
	 * The standard deviation is calculated as
	 * s = stdDev + d * stdDevSlope where d is the distance in centimeters.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar",
		Meta = (ClampMin = "0.0", EditCondition = "bEnableDistanceGaussianNoise"))
	FAGX_Real StandardDeviation {2.0};

	/**
	 * Standard deviation slope of the distance gaussian noise.
	 * The standard deviation is calculated as
	 * s = stdDev + d * stdDevSlope where d is the distance in centimeters.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar",
		Meta = (ClampMin = "0.0", EditCondition = "bEnableDistanceGaussianNoise"))
	FAGX_Real StandardDeviationSlope {0.0005};
};
