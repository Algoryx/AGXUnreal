// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

UENUM(BlueprintType)
enum class EAGX_TerrainWheelPressureSinkageModel : uint8
{
	Bekker,
	Reece
};
