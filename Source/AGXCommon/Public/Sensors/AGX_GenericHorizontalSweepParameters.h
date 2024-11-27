// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarModelParameters.h"

#include "AGX_GenericHorizontalSweepParameters.generated.h"

/**
 * Parameters used to create a Lidar Sensor Component using Lidar Model GenericHorizontalSweep.
 */
UCLASS(ClassGroup = "AGX_Sensor", Category = "AGX")
class AGXCOMMON_API UAGX_GenericHorizontalSweepParameters : public UAGX_LidarModelParameters
{
	GENERATED_BODY()

public:
	/**
	 * Field of View (FOV) of the Lidar Sensor [deg].
	 * The first element is the horizontal FOV, and the second vertical FOV.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar")
	FVector2D FOV {360.0, 40.0};

	/**
	 * Resolution, the smallest angle between two laser rays [deg].
	 * The first element is the horizontal FOV, and the second is vertical FOV.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar")
	FVector2D Resolution {1.0, 1.0};

	/**
	 * Determines how many times the total scan cycle is run per second, i.e. how often the total
	 * ray pattern of the Lidar Sensor is covered per second [Hz].
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar")
	double Frequency {5.0};
};
