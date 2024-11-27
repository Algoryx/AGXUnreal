// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_LidarModelParameters.generated.h"

/**
 * This asset defines the parameters used for a Lidar Sensor Component using a specific Lidar Model.
 */
UCLASS(ClassGroup = "AGX_Sensor", Category = "AGX", Abstract)
class AGXCOMMON_API UAGX_LidarModelParameters : public UObject
{
	GENERATED_BODY()
};
