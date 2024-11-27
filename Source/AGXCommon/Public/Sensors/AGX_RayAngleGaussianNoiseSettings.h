// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarEnums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_RayAngleGaussianNoiseSettings.generated.h"

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_RayAngleGaussianNoiseSettings
{
	GENERATED_BODY()

	/**
	 * The axis, local to the ray frame, around which the perturbation should be applied.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar")
	EAGX_LidarRayAngleDistortionAxis Axis {
		EAGX_LidarRayAngleDistortionAxis::AxisX};

	/**
	 * Mean of the ray angle gaussian noise [deg].
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar")
	double Mean {0.0};

	/**
	 * Standard deviation of the ray angle gaussian noise [deg].
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar", Meta = (ClampMin = "0.0"))
	double StandardDeviation {2.0};
};
