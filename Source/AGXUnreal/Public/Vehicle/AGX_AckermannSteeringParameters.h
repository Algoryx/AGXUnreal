// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Vehicle/AGX_SteeringParameters.h"

#include "AGX_AckermannSteeringParameters.generated.h"

/**
 * An asset used to hold configuration properties for the Ackermann steering mechanisms.
 */
UCLASS(ClassGroup = "AGX_Vehicle", Category = "AGX", BlueprintType)
class AGXUNREAL_API UAGX_AckermannSteeringParameters : public UAGX_SteeringParameters
{
	GENERATED_BODY()

public:
	UAGX_AckermannSteeringParameters();
};
