// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/CustomPatternFetcherBase.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

class UAGX_LidarSensorComponent;

class FAGX_CustomPatternFetcher : public FCustomPatternFetcherBase
{
public:
	virtual TArray<FTransform> GetRayTransforms() override;
	virtual FAGX_CustomPatternInterval GetNextInterval() override;

	void SetLidar(UAGX_LidarSensorComponent* InLidar);

private:
	TWeakObjectPtr<UAGX_LidarSensorComponent> Lidar;
};
