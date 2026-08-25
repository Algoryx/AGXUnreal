// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/CameraBackendPropagatorBase.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

class UAGX_CameraSensorComponent;
struct FCameraLensSingleElementParameters;

class FAGX_CameraBackendPropagator : public FCameraBackendPropagatorBase
{
public:
	virtual void OnBackendSetCameraLensSingleElement(
		const FCameraLensSingleElementParameters& Parameters) override;

	virtual void OnBackendRequestCapture(uint64 NativeOutputAddress) override;

	void SetCameraSensor(UAGX_CameraSensorComponent* InCameraSensor);
	UAGX_CameraSensorComponent* GetCameraSensor() const;

private:
	TWeakObjectPtr<UAGX_CameraSensorComponent> CameraSensor;
};
