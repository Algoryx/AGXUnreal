// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_CameraLensBase.h"

#include "AGX_CameraLensSingleElement.generated.h"

/**
 * AGX Camera single element lens asset.
 */
UCLASS(ClassGroup = "AGX_Sensor", Category = "AGX", BlueprintType)
class AGXUNREAL_API UAGX_CameraLensSingleElement : public UAGX_CameraLensBase
{
	GENERATED_BODY()

protected:
	virtual void CreateNative() override;
};
