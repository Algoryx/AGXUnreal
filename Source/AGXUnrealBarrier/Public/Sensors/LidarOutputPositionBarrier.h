// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/LidarOutputBarrier.h"

struct FAGX_LidarOutputPositionData;

class AGXUNREALBARRIER_API FLidarOutputPositionBarrier : public FLidarOutputBarrier
{
public:

	virtual void AllocateNative() override;

	void GetData(TArray<FAGX_LidarOutputPositionData>& OutData) const;
};
