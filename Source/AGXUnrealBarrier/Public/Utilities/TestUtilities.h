// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

struct FGuid;

/**
 * Convenience class made for internal Unit Tests to access parts of the plugin otherwise
 * unreachable from the AGXUnrealTest module.
 */
class AGXUNREALBARRIER_API FTestUtilities
{
public:
	static FString ConvertToAGXUuidStr(const FGuid& Guid);

	static FGuid ConvertAGXUuidToGuid(const FString& AGXUuidStr);
};
