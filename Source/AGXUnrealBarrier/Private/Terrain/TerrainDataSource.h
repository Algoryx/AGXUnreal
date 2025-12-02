// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxTerrain/TerrainDataSource.h>
#include "EndAGXIncludes.h"

class FTerrainHeightFetcherBase;

class FTerrainDataSource : public agxTerrain::ExternalTerrainDataSource
{
public:
	virtual agxTerrain::TerrainDataSource::TerrainHeightType fetchTerrainTile(
		const agxTerrain::TileSpecification& ts, agxTerrain::TileId id) override;

	void SetTerrainHeightFetcher(FTerrainHeightFetcherBase* HeightFetcher) noexcept;
	FTerrainHeightFetcherBase* GetTerrainHeightFetcher() const;

private:
	FTerrainHeightFetcherBase* HeightFetcher = nullptr;
};
