// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

struct FCameraOutputBarrier;
struct FCameraOutputColorBarrier;
struct FCameraLensSingleElementParameters;

class AGXUNREALBARRIER_API FCameraBackendPropagatorBase
{
public:
	virtual ~FCameraBackendPropagatorBase() = default;

	virtual void OnBackendSetCameraLensSingleElement(
		const FCameraLensSingleElementParameters& Parameters) = 0;
	virtual void OnBackendSetCameraColorOutput(
		const FCameraOutputColorBarrier& OutputColorBarrier) = 0;
	virtual void OnBackendRequestCapture(const FCameraOutputBarrier& OutputBarrier) = 0;
};
