// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_LidarSurfaceMaterial.generated.h"

/**
 * This asset represents a Surface Material that determines the interaction with Lidar laser rays as
 * they hit an object with this Surface Material assigned to it.
 */
UCLASS(ClassGroup = "AGX_Sensor", Category = "AGX", Abstract)
class AGXUNREAL_API UAGX_LidarSurfaceMaterial : public UObject
{
	GENERATED_BODY()

public:
	virtual UAGX_LidarSurfaceMaterial* GetOrCreateInstance(UWorld* PlayingWorld)
		PURE_VIRTUAL(UAGX_LidarSurfaceMaterial::GetOrCreateInstance, return nullptr;);
};
