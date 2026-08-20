// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_CameraBackendPropagator.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Sensors/AGX_CameraSensorComponent.h"

void FAGX_CameraBackendPropagator::OnBackendSetCameraLensSingleElement(
	const FCameraLensSingleElementParameters& Parameters)
{
	if (!CameraSensor.IsValid())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("OnBackendSetCameraLensSingleElement was called on a Camera Backend Propagator "
				 "with nullptr Camera Sensor."));
		return;
	}

	CameraSensor->OnBackendSetCameraLensSingleElement(Parameters);
}

void FAGX_CameraBackendPropagator::SetCameraSensor(
	UAGX_CameraSensorComponent* InCameraSensor)
{
	CameraSensor = InCameraSensor;
}

UAGX_CameraSensorComponent* FAGX_CameraBackendPropagator::GetCameraSensor() const
{
	return CameraSensor.Get();
}
