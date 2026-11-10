// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Terrain/LandscapeRendering/AGX_TerrainLandscapeRendererBaseComponent.h"

// Standard library includes.
#include <memory>

#include "AGX_TerrainPartitionedLandscapeRendererComponent.generated.h"

class UAGX_TerrainRuntimeVirtualTexture;
struct FAGX_TerrainRVTState;

/** Sparse RVT deformation for Nanite Landscapes, with or without World Partition. Requires Terrain
 * Paging. */
UCLASS(ClassGroup = "AGX_Terrain", Blueprintable, Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_TerrainPartitionedLandscapeRendererComponent
	: public UAGX_TerrainLandscapeRendererBaseComponent
{
	GENERATED_BODY()
public:
	virtual ~UAGX_TerrainPartitionedLandscapeRendererComponent();
	/** Signed range [-Range,+Range] [cm]. Fixed before play. Clipping is reported. */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Partitioned Landscape Rendering",
		Meta = (ClampMin = "1"))
	double DisplacementRange = 1000.0;

	/** Interior texels per RVT page: 64, 128 or 256. Fixed before play. Smaller pages reduce
	 * local update work. The shared GPU budget is set in Project Settings > Virtual Texture Pool
	 * for PF_G16, with physical tile size PageSize + 8 (including borders). */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Partitioned Landscape Rendering",
		Meta = (ClampMin = "64", ClampMax = "256"))
	int32 PageSize = 64;

	/** Number of sparse 64x64 deformation/mip blocks for active AGX tiles. */
	UPROPERTY(
		VisibleAnywhere, BlueprintReadOnly, Transient,
		Category = "AGX Partitioned Landscape Rendering|Statistics")
	int32 ResidentCPUBlocks = 0;

	/** Native tiles whose rendering data is currently retained, including pending source heights.
	 */
	UPROPERTY(
		VisibleAnywhere, BlueprintReadOnly, Transient,
		Category = "AGX Partitioned Landscape Rendering|Statistics")
	int32 ResidentCPUTiles = 0;

	UPROPERTY(
		VisibleAnywhere, BlueprintReadOnly, Transient,
		Category = "AGX Partitioned Landscape Rendering|Statistics")
	int32 PendingHeightSamples = 0;

	UPROPERTY(
		VisibleAnywhere, BlueprintReadOnly, Transient,
		Category = "AGX Partitioned Landscape Rendering|Statistics")
	int64 ChangedSamples = 0;

	UPROPERTY(
		VisibleAnywhere, BlueprintReadOnly, Transient,
		Category = "AGX Partitioned Landscape Rendering|Statistics")
	int64 ProducedPages = 0;

	UPROPERTY(
		VisibleAnywhere, BlueprintReadOnly, Transient,
		Category = "AGX Partitioned Landscape Rendering|Statistics")
	int32 VirtualResolution = 0;

	UPROPERTY(
		VisibleAnywhere, BlueprintReadOnly, Transient,
		Category = "AGX Partitioned Landscape Rendering|Statistics")
	int32 TileLoads = 0;

	UPROPERTY(
		VisibleAnywhere, BlueprintReadOnly, Transient,
		Category = "AGX Partitioned Landscape Rendering|Statistics")
	int32 TileUnloads = 0;

	UPROPERTY(
		VisibleAnywhere, BlueprintReadOnly, Transient,
		Category = "AGX Partitioned Landscape Rendering|Statistics")
	int32 ActiveTiles = 0;

	UFUNCTION(BlueprintPure, Category = "AGX Partitioned Landscape Rendering|Statistics")
	UAGX_TerrainRuntimeVirtualTexture* GetRuntimeVirtualTexture() const;

	/** Query active CPU deformation [cm]. GPU pages may still show cached deformation after CPU
	 * eviction. Persistent terrain state is owned by AGX. */
	UFUNCTION(BlueprintPure, Category = "AGX Partitioned Landscape Rendering|Statistics")
	double GetDisplacementAtLocation(FVector WorldPosition) const;

#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* Property) const override;
#endif
protected:
	virtual bool InitializeRenderer() override;
	virtual void UpdateRenderer(const TArray<FTerrainPagerHeightSample>& Samples) override;
	virtual void ShutdownRenderer() override;

private:
	void RefreshLandscapeProxies();
	UPROPERTY(Transient)
	TObjectPtr<UAGX_TerrainRuntimeVirtualTexture> TerrainRVTTexture;
	std::shared_ptr<FAGX_TerrainRVTState> RVTState;
};
