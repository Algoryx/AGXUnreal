// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Terrain/TerrainParticleTypes.h"

// Unreal Engine includes.
#include <CoreMinimal.h>

// Standard library includes.
#include <memory>

struct FRigidBodyBarrier;
struct FShapeBarrier;
class FShapeMaterialBarrier;
class FShovelBarrier;
class FTerrainBarrier;
class FTerrainHeightFetcherBase;
class FTerrainMaterialBarrier;

struct FTerrainDataSourceRef;
struct FTerrainPagerRef;

/** Active tile state in the pager frame, in centimeters and AGX row order. No native ownership. */
struct AGXUNREALBARRIER_API FTerrainTileHeights
{
	FIntPoint Id;
	FVector Corner;
	int32 SideVertices = 0;
	double Spacing = 0.0;
	bool bFullRefresh = false;
	TArray<float> Heights;
	TArray<FIntPoint> ModifiedVertices;
};

class AGXUNREALBARRIER_API FTerrainPagerBarrier
{
public:
	FTerrainPagerBarrier();
	FTerrainPagerBarrier(std::unique_ptr<FTerrainPagerRef> InNativeRef);
	FTerrainPagerBarrier(FTerrainPagerBarrier&& Other);
	~FTerrainPagerBarrier();

	bool HasNative() const;
	void AllocateNative(
		FTerrainBarrier& TerrainBarrier, int32 TileSideVertices, int32 TileOverlapVerties,
		double ElementSize, double MaxDepth);
	/** Caller keeps a raw fetcher alive until all native paging references are released. */
	void CreateTerrainDataSource(
		FTerrainHeightFetcherBase* HeightFetcher, FTerrainMaterialBarrier* DefaultTerrainMaterial);

	/** Keep source snapshots alive through outstanding native worker jobs and world teardown. */
	void CreateTerrainDataSource(
		TSharedRef<FTerrainHeightFetcherBase, ESPMode::ThreadSafe> HeightFetcher,
		FTerrainMaterialBarrier* DefaultTerrainMaterial);
	FTerrainPagerRef* GetNative();
	const FTerrainPagerRef* GetNative() const;
	void ReleaseNative();

	void SetCanCollide(bool bCanCollide);

	bool AddShovel(FShovelBarrier& Shovel, double RequiredRadius, double PreloadRadius);
	bool AddRigidBody(FRigidBodyBarrier& Body, double RequiredRadius, double PreloadRadius);

	bool SetTileLoadRadii(FRigidBodyBarrier& Body, double RequiredRadius, double PreloadRadius);

	/**
	* Assign Terrain Material for future terrain tiles overlapped by the given Shape.
	* The Shape is cloned before being given to the Native Terrain Data Source to allow
	* "stamping" where the same Shape is used to set Terrain Materials in many locations by
	* moving the Shape between calls.
	*/ 
	bool SetTerrainMaterial(FTerrainMaterialBarrier& TerrainMaterial, FShapeBarrier& Shape);
	bool SetAssociatedMaterial(
		FTerrainMaterialBarrier& TerrainMaterial, FShapeMaterialBarrier& ShapeMaterial);

	FParticleData GetParticleData() const;

	/**
	 * Get the data indicated by the ToInclude bit set flags of all particles.
	 *
	 * The resulting buffers are populated by entity ID, not by index, which means that there may be
	 * gaps in the data. An Exists array of bools indicate whether or not there is particle data at
	 * a particular index. The Exists array is always populated, even if ToInclude is all-zero.
	 */
	FParticleDataById GetParticleDataById(EParticleDataFlags ToInclude) const;

	/**
	 * Returns the total number of spawned Terrain particles.
	 */
	size_t GetNumParticles() const;

	/**
	 * Writes modified heights to OutHeights and returns an array of modified vertices, for easy
	 * iteration. The BoundVerts parameters is used to describe the vertex count of a grid that this
	 * native is assumed to be placed at its center, and that the vertex indices of this native will
	 * be mapped to.
	 */
	TArray<std::tuple<int32, int32>> GetModifiedHeights(
		TArray<float>& OutHeights, int32 BoundVertsX, int32 BoundVertsY) const;

	FVector GetReferencePoint() const;
	/** First vertex of tile (0,0), defining the simulation lattice independent of active tiles
	 * [world cm]. */
	FVector GetTileGridOrigin() const;
	FQuat GetReferenceRotation() const;

	TArray<FTransform> GetActiveTileTransforms() const;

	/**
	 * Read after stepping. New or restored tiles always return their complete authoritative state;
	 * known tiles return only changes. KnownTiles is pruned to the active set on every call.
	 */
	TArray<FTerrainTileHeights> GetTileHeightUpdates(TMap<FIntPoint, uint64>& KnownTiles) const;

	void OnTemplateTerrainChanged() const;

private:
	FTerrainPagerBarrier(const FTerrainPagerBarrier&) = delete;
	void operator=(const FTerrainPagerBarrier&) = delete;

	std::unique_ptr<FTerrainPagerRef> NativeRef;
	std::unique_ptr<FTerrainDataSourceRef> DataSourceRef;
};
