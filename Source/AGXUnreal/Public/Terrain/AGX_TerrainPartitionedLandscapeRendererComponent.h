// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "AGX_TerrainPartitionedLandscapeRendererComponent.generated.h"

class AAGX_Terrain;
class ALandscapeProxy;
class ULevel;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class UPrimitiveComponent;
class URuntimeVirtualTexture;
struct FAGX_TerrainSourceTile;
struct FAGX_TerrainVirtualTextureData;

USTRUCT()
struct FAGX_TerrainLandscapeMaterialBinding
{
	GENERATED_BODY()
	UPROPERTY()
	TWeakObjectPtr<UPrimitiveComponent> Component;
	UPROPERTY()
	TArray<TObjectPtr<UMaterialInterface>> OriginalMaterials;
	UPROPERTY()
	TArray<TObjectPtr<UMaterialInstanceDynamic>> Materials;
	float OriginalPositiveZBoundsExtension = 0.f;
	float OriginalNegativeZBoundsExtension = 0.f;
	uint8 OriginalShadowCacheInvalidation = 0;
};

/**
 * Bounded active-tile rendering through one fixed-coverage RVT and Nanite displacement.
 * Requires Terrain Paging, finite Terrain Bounds and cooked Nanite Landscape data.
 * Works with ordinary and World Partition Landscapes. It does not load streaming proxies.
 */
UCLASS(ClassGroup = "AGX_Terrain", Blueprintable, meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_TerrainPartitionedLandscapeRendererComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAGX_TerrainPartitionedLandscapeRendererComponent();
	virtual ~UAGX_TerrainPartitionedLandscapeRendererComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Terrain Rendering")
	bool bEnabled = true;

	/** Interior page side length [texels]. Borders add eight texels to the physical side. */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Terrain Rendering",
		meta = (ClampMin = "64", ClampMax = "1024"))
	int32 PageSize = 64;

	/** Maximum absolute vertical digging or piling displacement [cm]. Values outside this range
	 * clamp. */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Terrain Rendering",
		meta = (ClampMin = "1", Units = "cm"))
	double DisplacementRange = 500.0;

	/** Per-renderer page production limit. Does not change the project's global VT upload limits.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Terrain Rendering",
		meta = (ClampMin = "1", ClampMax = "256"))
	int32 MaxPagesPerFrame = 32;

	/** Parameter on the user's Landscape materials, of RVT content type Mask4, uncompressed. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Terrain Rendering")
	FName VirtualTextureParameter = TEXT("AGX_TerrainDisplacement");

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Rendering")
	void SetEnabled(bool bInEnabled);

	UFUNCTION(BlueprintPure, Category = "AGX Terrain Rendering")
	bool IsEnabled() const
	{
		return bEnabled;
	}

	UFUNCTION(BlueprintPure, Category = "AGX Terrain Rendering")
	URuntimeVirtualTexture* GetDisplacementVirtualTexture() const
	{
		return VirtualTexture;
	}

	UFUNCTION(BlueprintPure, Category = "AGX Terrain Rendering")
	int32 GetRetainedTileCount() const;

	UFUNCTION(BlueprintPure, Category = "AGX Terrain Rendering")
	int64 GetRetainedHeightBytes() const;

	/** Minimum and maximum retained displacement [cm], before encoding/clamping. */
	UFUNCTION(BlueprintPure, Category = "AGX Terrain Rendering")
	FVector2D GetActiveDisplacementExtrema() const;

	UFUNCTION(BlueprintPure, Category = "AGX Terrain Rendering")
	int64 GetGeneratedPageCount() const;

	/** CPU time of the most recent page-production batch [ms]. GPU cost is measured separately. */
	UFUNCTION(BlueprintPure, Category = "AGX Terrain Rendering")
	double GetLastPageProductionMilliseconds() const;

	void Initialize(AAGX_Terrain& InTerrain);
	void Update();
	void Shutdown();

#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* Property) const override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
#endif

protected:
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void TickComponent(
		float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* TickFunction) override;

private:
	void BindProxy(ALandscapeProxy& Proxy);
	void OnLevelAdded(ULevel* Level, UWorld* World);
	void OnLevelRemoved(ULevel* Level, UWorld* World);
	void Invalidate(const FVector2D& Min, const FVector2D& Max);
	void RequestPages();
	void RestoreMaterials(ULevel* Level = nullptr);
	UPROPERTY(Transient)
	URuntimeVirtualTexture* VirtualTexture = nullptr;
	UPROPERTY(Transient)
	TArray<FAGX_TerrainLandscapeMaterialBinding> Bindings;
	TMap<TWeakObjectPtr<ALandscapeProxy>, bool> OriginalDynamicMaterialFlags;
	TSharedPtr<FAGX_TerrainVirtualTextureData, ESPMode::ThreadSafe> Data;
	struct FTileState
	{
		TArray<float> Heights;
		TSharedPtr<const FAGX_TerrainSourceTile, ESPMode::ThreadSafe> Baseline;
		FVector Corner;
		int32 Side = 0;
		double Spacing = 0.0;
	};
	TMap<FIntPoint, FTileState> Tiles;
	TMap<FIntPoint, uint64> KnownTiles;
	AAGX_Terrain* Terrain = nullptr;
	FDelegateHandle LevelAddedHandle;
	FDelegateHandle LevelRemovedHandle;
	FDelegateHandle ActorSpawnedHandle;
	TArray<TWeakObjectPtr<ALandscapeProxy>> PendingProxies;
	bool bWarnedMaterial = false;
	bool bWarnedPreparation = false;
	bool bWarnedClipping = false;
};
