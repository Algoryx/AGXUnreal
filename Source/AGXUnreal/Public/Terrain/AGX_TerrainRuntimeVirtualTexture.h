// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "VT/RuntimeVirtualTexture.h"

#include "AGX_TerrainRuntimeVirtualTexture.generated.h"

/** G16 signed deformation storage. Runtime instances are owned by a single Terrain. */
UCLASS(BlueprintType)
class AGXUNREAL_API UAGX_TerrainRuntimeVirtualTexture : public URuntimeVirtualTexture
{
	GENERATED_BODY()

public:
	UAGX_TerrainRuntimeVirtualTexture(const FObjectInitializer& ObjectInitializer);
	void Configure(int32 TileCountLog2, int32 PageSize = 64);
};
