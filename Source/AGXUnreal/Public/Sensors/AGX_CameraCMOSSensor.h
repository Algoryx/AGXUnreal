// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_CameraPhotodetectorBase.h"

#include "AGX_CameraCMOSSensor.generated.h"

/**
 * AGX Camera CMOS Sensor photodetector asset.
 */
UCLASS(ClassGroup = "AGX_Sensor", Category = "AGX", BlueprintType)
class AGXUNREAL_API UAGX_CameraCMOSSensor : public UAGX_CameraPhotodetectorBase
{
	GENERATED_BODY()

protected:
	virtual void CreateNative() override;
};
