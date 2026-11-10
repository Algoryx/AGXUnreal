// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "Math/Box.h"

// Standard library includes.
#include <memory>

#include "AGX_TerrainRVTComponent.generated.h"

class ALandscape;
class FTerrainPagerBarrier;
class UAGX_TerrainRuntimeVirtualTexture;
struct FAGX_TerrainRVTState;

/** Experimental direct virtual-texture rendering for Landscape, including streamed proxies. */
UCLASS(ClassGroup = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_TerrainRVTComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAGX_TerrainRVTComponent();
	virtual ~UAGX_TerrainRVTComponent();

	/** Opt in to sparse paging and direct RVT rendering. Requires Terrain Paging. Fixed before
	 * play. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Terrain RVT")
	bool bEnabled = false;

	/** Native Terrain sample spacing [cm], independent of Landscape. Fixed before play. */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Terrain RVT", Meta = (ClampMin = "1"))
	double TerrainVertexSpacing = 25.0;

	/** Signed range [-Range,+Range] [cm]. Fixed before play. Clipping is reported. */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Terrain RVT", Meta = (ClampMin = "1"))
	double DisplacementRange = 1000.0;

	/** Interior texels per RVT page: 64, 128 or 256. Fixed before play. Smaller pages reduce
	 * local update work. The shared GPU budget is set in Project Settings > Virtual Texture Pool
	 * for PF_G16, with physical tile size PageSize + 8 (including borders). */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Terrain RVT",
		Meta = (ClampMin = "64", ClampMax = "256"))
	int32 PageSize = 64;

	/** Source snapshots retained for loaded components or active AGX tiles. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "AGX Terrain RVT")
	int32 SourceSnapshotCount = 0;

	/** Requests rejected because some source heights were unavailable. Retried by AGX. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "AGX Terrain RVT")
	int64 UnavailableHeightRequests = 0;

	/** Number of sparse 64x64 deformation/mip blocks for active AGX tiles. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "AGX Terrain RVT")
	int32 ResidentCPUBlocks = 0;

	/** Native tiles whose rendering data is currently retained, including pending source heights.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "AGX Terrain RVT")
	int32 ResidentCPUTiles = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "AGX Terrain RVT")
	int32 PendingHeightSamples = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "AGX Terrain RVT")
	int64 ChangedSamples = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "AGX Terrain RVT")
	int64 ProducedPages = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "AGX Terrain RVT")
	int32 VirtualResolution = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "AGX Terrain RVT")
	int32 TileLoads = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "AGX Terrain RVT")
	int32 TileUnloads = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "AGX Terrain RVT")
	int32 ActiveTiles = 0;

	UFUNCTION(BlueprintPure, Category = "AGX Terrain RVT")
	UAGX_TerrainRuntimeVirtualTexture* GetRuntimeVirtualTexture() const;

	/** Query active CPU deformation [cm]. GPU pages may still show cached deformation after CPU
	 * eviction. Persistent terrain state is owned by AGX. */
	UFUNCTION(BlueprintPure, Category = "AGX Terrain RVT")
	double GetDisplacementAtLocation(FVector WorldPosition) const;

	/** Coverage is the finite TerrainBoundsComponent box, fixed before play. */
	bool Initialize(ALandscape& Landscape, const FBox& Coverage);
	FBox GetSourceBounds() const;
	virtual void TickComponent(
		float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;
	void SetTerrainBounds(const FVector& Origin, const FVector& HalfExtent);
	bool FetchHeights(
		const FVector& Start, int32 VertsX, int32 VertsY, TArray<float>& Heights) const;
	void Update(FTerrainPagerBarrier& Pager);
	void Shutdown();

#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* Property) const override;
#endif

private:
	void RefreshLandscapeProxies();
	void PruneSourceSnapshots();

	// Retained only to deserialize the earlier experimental assets. TerrainBounds now defines
	// coverage. These settings are deliberately no longer used for runtime bounds.
	UPROPERTY()
	bool bUseFixedLandscapeBounds_DEPRECATED = false;
	UPROPERTY()
	FVector2D LandscapeBoundsMin_DEPRECATED = FVector2D::ZeroVector;
	UPROPERTY()
	FVector2D LandscapeBoundsMax_DEPRECATED = FVector2D::ZeroVector;

	UPROPERTY(Transient)
	TObjectPtr<UAGX_TerrainRuntimeVirtualTexture> Texture;

	std::shared_ptr<FAGX_TerrainRVTState> State;
};
