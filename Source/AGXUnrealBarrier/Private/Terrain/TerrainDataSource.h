// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Templates/SharedPointer.h"

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

	void SetOwnedTerrainHeightFetcher(
		TSharedRef<FTerrainHeightFetcherBase, ESPMode::ThreadSafe> InHeightFetcher);

private:
	FTerrainHeightFetcherBase* HeightFetcher = nullptr;
	TSharedPtr<FTerrainHeightFetcherBase, ESPMode::ThreadSafe> OwnedHeightFetcher;
};
