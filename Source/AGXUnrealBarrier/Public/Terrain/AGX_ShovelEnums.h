// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

#include "AGX_ShovelEnums.generated.h"

UENUM(BlueprintType)
enum class EAGX_ExcavationMode : uint8
{
	Primary,
	DeformBack,
	DeformRight,
	DeformLeft
};
