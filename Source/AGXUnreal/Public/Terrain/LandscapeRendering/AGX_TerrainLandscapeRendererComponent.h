// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Terrain/LandscapeRendering/AGX_TerrainLandscapeRendererBaseComponent.h"

// Unreal Engine includes.
#include "DataDrivenShaderPlatformInfo.h"
#include "Engine/TextureRenderTarget2D.h"

#include "AGX_TerrainLandscapeRendererComponent.generated.h"

/** Traditional Landscape deformation using a render target at Landscape resolution. */
UCLASS(ClassGroup = "AGX_Terrain", Blueprintable, Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_TerrainLandscapeRendererComponent
	: public UAGX_TerrainLandscapeRendererBaseComponent
{
	GENERATED_BODY()
public:
	UAGX_TerrainLandscapeRendererComponent();
	// TODO: Should try finding this from the material automatically!
	/**
	 * The Displacement Map Render Target which AGX Terrain will write height changes to.
	 *
	 * Requirements:
	 *
	 * 1. Used as Displacement Map for the AGX Landscape Material.
	 * 2. Render Target Format must be "R16f".
	 * 3. SizeX and SizeY must equal number of Landscape Vertices in respective dimension (Quads +
	 * 1).
	 * 4. Texture Address Mode should be Clamp.
	 * 5. No Mip Maps.
	 * 6. Texture Group should preferrably be either "RenderTarget" for smooth results,
	 *    or "2D Pixels (unfiltered)" for more precise results.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Landscape Rendering", Meta = (EditCondition = "bEnabled"))
	UTextureRenderTarget2D* LandscapeDisplacementMap;

	/** Retained legacy shovel active-zone visualization setting. */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Debug Rendering")
	bool bEnableActiveZoneRendering = false;
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* Property) const override;
#endif
protected:
	virtual bool InitializeRenderer() override;
	virtual void UpdateRenderer(const TArray<FTerrainPagerHeightSample>& Samples) override;
	virtual void ShutdownRenderer() override;

private:
	void InitializeDisplacementMap();
	void UpdateLandscapeMaterialParameters();
	void Upload();
	void ClearDisplacementMap();
	TArray<float> OriginalHeights;
	TArray<float> CurrentHeights;
	TArray<FFloat16> DisplacementData;
	TArray<FUpdateTextureRegion2D> DisplacementMapRegions;
	int32 NumVerticesX = 0;
	int32 NumVerticesY = 0;
	bool DisplacementMapInitialized = false;
};
