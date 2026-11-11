// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "VirtualTexturing.h"

// Standard library includes.
#include <atomic>

/** Render-owned active AGX tiles. Commands publish changes; page generation never locks gameplay.
 */
struct FAGX_TerrainVirtualTextureData
{
	struct FTile
	{
		FIntPoint Id;
		FVector2D Corner;
		double Spacing = 0.0;
		int32 Side = 0;
		TArray<float> Heights;
		struct FMip
		{
			int32 Side = 0;
			TArray<float> Heights;
		};
		TArray<FMip> Mips;

		/** Empty vertices rebuilds all levels. Otherwise update only affected parent samples. */
		AGXUNREAL_API void UpdateMips(TConstArrayView<FIntPoint> Vertices = {});
	};
	TMap<FIntPoint, FTile> Tiles;
	FTransform Frame;
	FVector2D CoverageMin;
	FVector2D CoverageSize;
	FVector2D TerrainHalfExtent;
	int32 MaxPagesPerFrame = 32;
	std::atomic<int64> ProducedPages {0};
	std::atomic<int64> RetainedHeightBytes {0};
	std::atomic<double> LastProductionMilliseconds {0.0};

	/** Height [cm], dZ/dX, dZ/dY, validity. Footprints > native spacing use box-filtered mips. */
	AGXUNREAL_API FVector4f Sample(
		const FVector2D& Position, TConstArrayView<const FTile*> Candidates,
		double Footprint = 0.0) const;
};

IVirtualTexture* AGX_CreateTerrainVirtualTextureProducer(
	TSharedRef<FAGX_TerrainVirtualTextureData, ESPMode::ThreadSafe> Data,
	const FVTProducerDescription& Description);
