// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/CameraBackendPropagatorBase.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

class UAGX_CameraSensorComponent;
struct FCameraCMOSSensorBarrier;
struct FCameraLensSingleElementBarrier;
struct FCameraOutputColorBarrier;

class FAGX_CameraBackendPropagator : public FCameraBackendPropagatorBase
{
public:
	virtual void OnBackendSetCameraLensSingleElement(
		const FCameraLensSingleElementBarrier& LensBarrier) override;

	virtual void OnBackendSetCameraCMOSSensor(
		const FCameraCMOSSensorBarrier& SensorBarrier) override;

	virtual void OnBackendSetCameraColorOutput(
		const FCameraOutputColorBarrier& OutputColorBarrier) override;

	virtual void OnBackendRequestCapture(const FCameraOutputBarrier& OutputBarrier) override;

	void SetCameraSensor(UAGX_CameraSensorComponent* InCameraSensor);
	UAGX_CameraSensorComponent* GetCameraSensor() const;

private:
	TWeakObjectPtr<UAGX_CameraSensorComponent> CameraSensor;
};
