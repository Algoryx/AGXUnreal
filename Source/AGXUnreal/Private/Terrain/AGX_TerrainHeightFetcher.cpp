// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainHeightFetcher.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Terrain/AGX_Terrain.h"

bool FAGX_TerrainHeightFetcher::FetchHeights(
	const FVector& WorldPosStart, int32 VertsX, int32 VertsY, TArray<float>& OutHeights) const
{
	if (Terrain == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("FetchHeights called on a TerrainHeightFetcher with nullptr Terrain. Fetching "
				 "heights will not be possible."));
		return false;
	}

	return Terrain->FetchHeights(WorldPosStart, VertsX, VertsY, OutHeights);
}

void FAGX_TerrainHeightFetcher::SetTerrain(AAGX_Terrain* InTerrain)
{
	Terrain = InTerrain;
}

AAGX_Terrain* FAGX_TerrainHeightFetcher::GetTerrain() const
{
	return Terrain;
}
