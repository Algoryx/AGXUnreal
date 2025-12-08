// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/TerrainDataSource.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Terrain/TerrainHeightFetcherBase.h"
#include "TypeConversions.h"

agxTerrain::TerrainDataSource::TerrainHeightType FTerrainDataSource::fetchTerrainTile(
	const agxTerrain::TileSpecification& ts, agxTerrain::TileId id)
{
	TerrainHeightType HeightsAGX;

	if (HeightFetcher == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("fetchTerrainTile called on a TerrainDataSource that does not have a "
				 "HeightFetcher set. Fetchin Terrain Tile will fail."));
		return HeightsAGX;
	}

	const agx::Real TileSizeHalfAGX = ts.getTileSize() / 2.0;
	const FVector TileCornerWorldPosUnreal = ConvertDisplacement(
		ts.convertTilePositionToWorld(id, agx::Vec2(-TileSizeHalfAGX, TileSizeHalfAGX)));
	const int32 VertCountSide = static_cast<int32>(ts.getTileResolution());

	TArray<float> Heights;
	if (!HeightFetcher->FetchHeights(
			TileCornerWorldPosUnreal, VertCountSide, VertCountSide, Heights))
	{
		return HeightsAGX;
	}

	HeightsAGX.reserve(Heights.Num());
	for (float Height : Heights)
	{
		HeightsAGX.push_back(ConvertDistanceToAGX(Height));
	}

	return HeightsAGX;
}

void FTerrainDataSource::SetTerrainHeightFetcher(
	FTerrainHeightFetcherBase* InHeightFetcher) noexcept
{
	HeightFetcher = InHeightFetcher;
}

FTerrainHeightFetcherBase* FTerrainDataSource::GetTerrainHeightFetcher() const
{
	return HeightFetcher;
}
