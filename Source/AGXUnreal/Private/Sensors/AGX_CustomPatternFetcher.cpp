// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_CustomPatternFetcher.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Sensors/AGX_LidarSensorComponent.h"

TArray<FTransform> FAGX_CustomPatternFetcher::GetRayTransforms()
{
	if (Lidar == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("GetRayTransforms was called on FAGX_CustomPatternFetcher that does not have a "
				 "Lidar associated with it. Custom Pattern will not work."));
		return TArray<FTransform>();
	}

	return Lidar->FetchRayTransforms();
}

FAGX_CustomPatternInterval FAGX_CustomPatternFetcher::GetNextInterval()
{
	if (Lidar == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("GetNextInterval was called on FAGX_CustomPatternFetcher that does not have a "
				 "Lidar associated with it. Custom Pattern will not work."));
		return FAGX_CustomPatternInterval();
	}
	
	return Lidar->FetchNextInterval();
}

void FAGX_CustomPatternFetcher::SetLidar(UAGX_LidarSensorComponent* InLidar)
{
	Lidar = InLidar;
}
