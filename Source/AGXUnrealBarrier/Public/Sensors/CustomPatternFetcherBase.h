// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_CustomPatternInterval.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

class AGXUNREALBARRIER_API FCustomPatternFetcherBase
{
public:
	virtual TArray<FTransform> GetRayTransforms() = 0;

	virtual FAGX_CustomPatternInterval GetNextInterval() = 0;
};
