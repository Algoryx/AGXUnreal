// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Math/Float16.h"
#include "RHI.h"

#include "AGX_TerrainLandscapeRendererComponent.generated.h"

class AAGX_Terrain;
class UTextureRenderTarget2D;

/** Released rendering for non-partitioned Landscapes using a dense R16F render target.
 * Terrain Paging is supported on ordinary Landscapes. Partitioned sources require
 * UAGX_TerrainPartitionedLandscapeRendererComponent instead.
 */
UCLASS(ClassGroup = "AGX_Terrain", Blueprintable, meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_TerrainLandscapeRendererComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAGX_TerrainLandscapeRendererComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Terrain Rendering")
	bool bEnabled = true;

	/** R16F displacement render target used by the Landscape material [cm]. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Terrain Rendering")
	UTextureRenderTarget2D* LandscapeDisplacementMap;

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Rendering")
	void SetEnabled(bool bInEnabled);

	UFUNCTION(BlueprintPure, Category = "AGX Terrain Rendering")
	bool IsEnabled() const
	{
		return bEnabled;
	}

	void Initialize(AAGX_Terrain& InTerrain);
	void Update();
	void Shutdown();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void Serialize(FArchive& Archive) override;

private:
	friend class AAGX_Terrain;
	bool ValidateSource(const AAGX_Terrain& InTerrain);
	void InitializeDisplacementMap();
	void UpdateLandscapeMaterialParameters();
	void UpdateDisplacementMap();
	void ClearDisplacementMap();
	AAGX_Terrain* Terrain = nullptr;
	TMap<FIntPoint, uint64> KnownTiles;
	TArray<float> OriginalHeights;
	TArray<float> CurrentHeights;
	TArray<FFloat16> DisplacementData;
	TArray<FUpdateTextureRegion2D> DisplacementMapRegions;
	int32 NumVerticesX = 0;
	int32 NumVerticesY = 0;
	bool DisplacementMapInitialized = false;
	bool bUnsupportedSourceReported = false;
};
