// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

struct FCameraOutputBarrier;
struct FCameraOutputColorBarrier;
struct FCameraCMOSSensorBarrier;
struct FCameraLensSingleElementBarrier;

class AGXUNREALBARRIER_API FCameraBackendPropagatorBase
{
public:
	virtual ~FCameraBackendPropagatorBase() = default;

	virtual void OnBackendSetCameraLensSingleElement(
		const FCameraLensSingleElementBarrier& LensBarrier) = 0;
	virtual void OnBackendSetCameraCMOSSensor(
		const FCameraCMOSSensorBarrier& SensorBarrier) = 0;
	virtual void OnBackendSetCameraColorOutput(
		const FCameraOutputColorBarrier& OutputColorBarrier) = 0;
	virtual void OnBackendRequestCapture(const FCameraOutputBarrier& OutputBarrier) = 0;
};
