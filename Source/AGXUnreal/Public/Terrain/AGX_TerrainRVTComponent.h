// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Components/ActorComponent.h"

// Standard library includes.
#include <memory>

#include "AGX_TerrainRVTComponent.generated.h"

class ALandscape;
class FTerrainPagerBarrier;
class UAGX_TerrainRuntimeVirtualTexture;
struct FAGX_TerrainRVTState;

/** Experimental direct virtual-texture rendering for a loaded, non-partitioned Landscape. */
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

	/** Number of sparse 64x64 deformation blocks retained for this play session. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "AGX Terrain RVT")
	int32 ResidentCPUBlocks = 0;

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

	/** Query retained signed deformation [cm]. Also useful for runtime diagnostics. */
	UFUNCTION(BlueprintPure, Category = "AGX Terrain RVT")
	double GetDisplacementAtLocation(FVector WorldPosition) const;

	bool Initialize(ALandscape& Landscape);
	void SetTerrainBounds(const FVector& Origin, const FVector& HalfExtent);
	bool FetchHeights(
		const FVector& Start, int32 VertsX, int32 VertsY, TArray<float>& Heights) const;
	void Update(FTerrainPagerBarrier& Pager);
	void Shutdown();

#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* Property) const override;
#endif

private:
	UPROPERTY(Transient)
	TObjectPtr<UAGX_TerrainRuntimeVirtualTexture> Texture;

	std::shared_ptr<FAGX_TerrainRVTState> State;
};
