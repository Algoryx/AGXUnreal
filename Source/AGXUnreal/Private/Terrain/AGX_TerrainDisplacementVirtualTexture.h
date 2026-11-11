// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "VT/RuntimeVirtualTexture.h"

#include "AGX_TerrainDisplacementVirtualTexture.generated.h"

/** Transient runtime RVT configuration. Material assets reference an ordinary Mask4 RVT. */
UCLASS(Transient)
class UAGX_TerrainDisplacementVirtualTexture : public URuntimeVirtualTexture
{
	GENERATED_BODY()

public:
	void Configure(int32 PageSize, int32 TileCountLog2)
	{
		MaterialType = ERuntimeVirtualTextureMaterialType::Mask4;
		bCompressTextures = false;
		TileSize = FMath::FloorLog2(PageSize) - 6;
		TileCount = TileCountLog2;
		TileBorderSize = 2;
		bAdaptive = false;
		bPrivateSpace = true;
		bSinglePhysicalSpace = true;
	}
};
