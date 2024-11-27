// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarSurfaceMaterial.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Engine/AssetUserData.h"

#include "AGX_SurfaceMaterialAssetUserData.generated.h"

UCLASS(BlueprintType, ClassGroup = "AGX", Category = "AGX")
class AGXUNREAL_API UAGX_SurfaceMaterialAssetUserData : public UAssetUserData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<UAGX_LidarSurfaceMaterial> LidarSurfaceMaterial {nullptr};
};
