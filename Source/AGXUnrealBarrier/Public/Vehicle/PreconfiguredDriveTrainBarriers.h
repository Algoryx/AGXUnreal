// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "DriveTrain/CombustionEngineBarrier.h"
#include "PowerLine/PowerLineBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

struct AGXUNREALBARRIER_API FPreconfiguredDriveTrainBarriers
{
	FPowerLineBarrier PowerLine;
	FCombustionEngineBarrier CombustionEngine;
};
