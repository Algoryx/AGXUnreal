// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_DistanceGaussianNoiseSettings.generated.h"

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_DistanceGaussianNoiseSettings
{
	GENERATED_BODY()

	/**
	 * Mean of the distance gaussian noise [cm].
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar")
	double Mean {0.0};

	/**
	 * Standard deviation of the distance gaussian noise [cm].
	 * The standard deviation is calculated as
	 * s = stdDev + d * stdDevSlope where d is the distance in centimeters.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar", Meta = (ClampMin = "0.0"))
	double StandardDeviation {2.0};

	/**
	 * Standard deviation slope of the distance gaussian noise.
	 * The standard deviation is calculated as
	 * s = stdDev + d * stdDevSlope where d is the distance in centimeters.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar")
	double StandardDeviationSlope {0.0005};
};
