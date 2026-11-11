// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Terrain/TerrainHeightFetcherBase.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <mutex>

class AAGX_Terrain;
class ALandscape;

/** Immutable source heights, in AGX row order, measured along the Landscape Z axis [cm]. */
struct FAGX_TerrainSourceTile
{
	TArray<float> Heights;
	FVector Corner;
	int32 SizeX = 0;
	int32 SizeY = 0;
};

class AGXUNREAL_API FAGX_TerrainHeightFetcher : public FTerrainHeightFetcherBase
{
public:
	FAGX_TerrainHeightFetcher();
	virtual ~FAGX_TerrainHeightFetcher();
	virtual bool FetchHeights(
		const FVector& WorldPosStart, int32 VertsX, int32 VertsY,
		TArray<float>& OutHeights) const override;

	void SetTerrain(AAGX_Terrain* Terrain);
	AAGX_Terrain* GetTerrain() const;

	/** Configure before native workers start. No UObject is accessed by snapshot reads. */
	void ConfigureSnapshots(const FTransform& Frame, const FVector2D& HalfExtent, double Spacing);
	void ServiceRequests(ALandscape& Landscape);
	TSharedPtr<const FAGX_TerrainSourceTile, ESPMode::ThreadSafe> GetOrRequestSnapshot(
		const FVector& Corner, int32 SizeX, int32 SizeY) const;
	void ResetSnapshots();
	int32 GetSnapshotCount() const;

private:
	AAGX_Terrain* Terrain = nullptr;
	struct FRequest
	{
		FVector Corner;
		int32 SizeX;
		int32 SizeY;
		uint64 LastUsedFrame;
		TSharedPtr<const FAGX_TerrainSourceTile, ESPMode::ThreadSafe> Snapshot;
	};
	mutable std::mutex SnapshotMutex;
	mutable TMap<FIntPoint, FRequest> Requests;
	// One bounded asynchronous resampling job; it owns numeric data, never this fetcher.
	struct FSourceCache;
	TUniquePtr<FSourceCache> SourceCache;
	uint64 ServiceFrame = 0;
	uint64 LastServiceFrame = MAX_uint64;
	FTransform SnapshotFrame;
	FVector2D SnapshotHalfExtent = FVector2D::ZeroVector;
	double SnapshotSpacing = 0.0;
	bool bUseSnapshots = false;
	// These limits bound missing-data work as well as recently consumed source snapshots.
	static constexpr int32 MaxRequests = 256;
	static constexpr int32 MaxVerticesPerTile = 1024 * 1024;
	static constexpr int64 MaxSourceSnapshotBytes = 32 * 1024 * 1024;
};
