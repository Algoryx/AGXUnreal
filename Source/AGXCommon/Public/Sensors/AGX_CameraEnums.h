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
	I8,
	U8,
	I16,
	U16,
	I32,
	U32,
	F32,
	I64,
	U64,
	F64
};
