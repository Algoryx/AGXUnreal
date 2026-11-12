// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Terrain/AGX_TerrainHeightFetcher.h"
#include "Terrain/TerrainPagerBarrier.h"

class FTerrainBarrier;

/** Game-thread adapter from a fixed native heightfield to the RVT's small square chunks.
 * The initial baseline survives renderer disable/enable for the native terrain's lifetime.
 */
class AGXUNREAL_API FAGX_TerrainRVTHeightSource
{
public:
	FAGX_TerrainRVTHeightSource(const FTerrainBarrier& Native, const FTransform& InFrame);
	TArray<FTerrainTileHeights> GetTileHeightUpdates(
		const FTerrainBarrier& Native, TMap<FIntPoint, uint64>& KnownTiles);
	TSharedPtr<const FAGX_TerrainSourceTile, ESPMode::ThreadSafe> GetSourceTileSnapshot(
		const FVector& Corner, int32 Side) const;
	int64 GetRetainedBytes() const;

private:
	FTerrainTileHeights MakeTile(const FIntPoint& Id, bool bFullRefresh) const;
	float Sample(const TArray<float>& Heights, int32 X, int32 Y) const;
	FIntPoint GetChunkCount() const;

	// One-cell halos provide forward gradients on both sides of chunk seams.
	static constexpr int32 ChunkQuads = 64;
	static constexpr int32 Halo = 1;
	static constexpr int32 ChunkSide = ChunkQuads + 1 + 2 * Halo;
	FTransform Frame;
	FIntPoint GridSize;
	FVector2D HalfSize;
	double Spacing = 0;
	TArray<float> InitialHeights;
	TArray<float> CurrentHeights;
};
