// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

#include "AGX_CameraEnums.generated.h"

/** Specifies the output channel element type for a Camera Output. */
UENUM(BlueprintType)
enum class EAGX_CameraOutputChannelType : uint8
{
	UNSUPPORTED,
	U8,
	F32
};
