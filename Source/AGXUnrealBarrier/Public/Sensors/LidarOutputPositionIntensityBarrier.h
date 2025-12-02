// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/LidarOutputBarrier.h"

struct FAGX_LidarOutputPositionIntensityData;

class AGXUNREALBARRIER_API FLidarOutputPositionIntensityBarrier : public FLidarOutputBarrier
{
public:

	virtual void AllocateNative() override;

	void GetData(TArray<FAGX_LidarOutputPositionIntensityData>& OutData) const;
};
