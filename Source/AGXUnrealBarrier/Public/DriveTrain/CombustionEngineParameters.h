// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "CombustionEngineParameters.generated.h"

USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FAGX_CombustionEngineParameters
{
	GENERATED_BODY()

	/**
	 * The total displacement volume of the engine [cm^3]
	 *
	 * Is the sum of the volumes of the cylinders.
	 */
	UPROPERTY(VisibleAnywhere, Category = "AGX Combustion Engine")
	FAGX_Real DisplacementVolume {1969.0};
};
