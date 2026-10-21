// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarModelParameters.h"

#include "AGX_LivoxAviaParameters.generated.h"

/**
 * Parameters used to create a Lidar Sensor Component using Lidar Model LivoxAvia.
 */
UCLASS(ClassGroup = "AGX_Sensor", Category = "AGX")
class AGXCOMMON_API UAGX_LivoxAviaParameters : public UAGX_LidarModelParameters
{
	GENERATED_BODY()

public:
	/**
	 * Divides the number of rays emitted per frame. 1 uses the full output rate,
	 * 2 uses half the output rate, and so on. Must be at least 1.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Lidar", meta = (ClampMin = "1"))
	int32 Downsample {1};
};
