// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarModelParameters.h"

#include "AGX_CustomRayPatternParameters.generated.h"

/**
 * Parameters used to create a Lidar Sensor Component using Lidar Model CustomRayPattern.
 */
UCLASS(ClassGroup = "AGX_Sensor", Category = "AGX")
class AGXCOMMON_API UAGX_CustomRayPatternParameters : public UAGX_LidarModelParameters
{
	GENERATED_BODY()
};
