// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

#include "AGX_CableEnums.generated.h"

UENUM(BlueprintType)
enum class EAGX_CableNodeType : uint8
{
	Free,
	BodyFixed,
	NumNodeTypes UMETA(HIDDEN)
};
