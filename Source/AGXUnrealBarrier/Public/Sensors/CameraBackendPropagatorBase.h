// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

struct FCameraLensSingleElementParametersBarrier;

class AGXUNREALBARRIER_API FCameraBackendPropagatorBase
{
public:
	virtual ~FCameraBackendPropagatorBase() = default;

	virtual void OnBackendSetCameraLensSingleElement(
		FCameraLensSingleElementParametersBarrier& Parameters) = 0;
};
