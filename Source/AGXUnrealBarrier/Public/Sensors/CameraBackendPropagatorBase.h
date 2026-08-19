// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

struct FCameraLensSingleElementParameters;

class AGXUNREALBARRIER_API FCameraBackendPropagatorBase
{
public:
	virtual ~FCameraBackendPropagatorBase() = default;

	virtual void OnBackendSetCameraLensSingleElement(
		const FCameraLensSingleElementParameters& Parameters) = 0;
};
