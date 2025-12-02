// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

class AGXUNREALBARRIER_API FTerrainHeightFetcherBase
{
public:
	/**
	 * Reads heights in a grid from a starting position. VertsX x VertsY number of heights are read
	 * and written to the out variable.
	 * Returns true if heights could be read successfully, false otherwise.
	 */
	virtual bool FetchHeights(
		const FVector& WorldPosStart, int32 VertsX, int32 VertsY,
		TArray<float>& OutHeights) const = 0;
};
