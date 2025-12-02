// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarEnums.h"
#include "Sensors/AGX_LidarModelParameters.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

class AGXUNREAL_API FAGX_SensorUtilities
{
public:
	static UClass* GetParameterTypeFrom(EAGX_LidarModel Model);
};
