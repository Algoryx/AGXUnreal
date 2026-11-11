// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "HAL/CriticalSection.h"
#include "VirtualTexturing.h"

// Standard library includes.
#include <atomic>

/** Only the active AGX tile working set. Shared with the producer; contains no UObjects. */
struct FAGX_TerrainVirtualTextureData
{
	struct FTile
	{
		FIntPoint Id;
		FVector2D Corner;
		double Spacing = 0.0;
		int32 Side = 0;
		TArray<float> Heights;
	};
	FRWLock Lock;
	TMap<FIntPoint, FTile> Tiles;
	FTransform Frame;
	FVector2D CoverageMin;
	FVector2D CoverageSize;
	int32 MaxPagesPerFrame = 32;
	std::atomic<int64> ProducedPages {0};
	std::atomic<double> LastProductionMilliseconds {0.0};

	/** Current height [cm], plane derivatives dZ/dX and dZ/dY, validity. Caller holds a read lock.
	 */
	AGXUNREAL_API FVector4f
	Sample(const FVector2D& Position, TConstArrayView<const FTile*> Candidates) const;
};

IVirtualTexture* AGX_CreateTerrainVirtualTextureProducer(
	TSharedRef<FAGX_TerrainVirtualTextureData, ESPMode::ThreadSafe> Data,
	const FVTProducerDescription& Description);
