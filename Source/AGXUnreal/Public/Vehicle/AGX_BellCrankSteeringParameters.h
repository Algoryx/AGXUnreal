// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Vehicle/AGX_SteeringParameters.h"

#include "AGX_BellCrankSteeringParameters.generated.h"

/**
 * An asset used to hold configuration properties for the BellCrank steering mechanisms.
 */
UCLASS(ClassGroup = "AGX_Vehicle", Category = "AGX", BlueprintType)
class AGXUNREAL_API UAGX_BellCrankSteeringParameters : public UAGX_SteeringParameters
{
	GENERATED_BODY()

public:
	UAGX_BellCrankSteeringParameters();
};
