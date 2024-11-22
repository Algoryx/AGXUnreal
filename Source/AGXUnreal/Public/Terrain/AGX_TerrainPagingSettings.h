// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes
#include "AGX_TerrainPagingBodyReference.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Math/Color.h"

#include "AGX_TerrainPagingSettings.generated.h"

USTRUCT()
struct AGXUNREAL_API FAGX_TerrainPagingSettings
{
	GENERATED_BODY()

	/**
	 * Specifies the overlap between Terrain tiles [cm].
	 * The overlap should be larger than both the length of the cutting edge interacting with the
	 * Terrain as well as twice the expected maximum length of a soil-wedge (which depends on how
	 * deep excavation in the terrain will be performed).
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Paging Settings")
	double TileOverlap {500.0};

	/**
	 * The overall size (side length), including overlap, of each Terrain tile [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Paging Settings")
	double TileSize {2500.0};

	/**
	 * Specifies whether or not to draw Terrain Paging grid debug rendering.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Paging Settings")
	bool bDrawDebugGrid {true};

	/**
	 * Specifies whether or not to draw Terrain Paging load radii debug rendering.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Paging Settings")
	bool bDrawDebugLoadRadii {true};

#if WITH_EDITORONLY_DATA
	/** Color of the lines showing tile boundaries in the editor. */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Paging Editor Settings")
	FLinearColor TileOutlineColor {FLinearColor::Gray};

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Paging Editor Settings")
	FLinearColor LoadedTileOutlineColor {FLinearColor::White};

	/** Width of the lines showing tile boundaries in the editor. Loaded tile lines are thicker. */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Paging Editor Settings")
	float TileOutlineThickness {4.0};
#endif

	/**
	 * Rigid Bodies tracked by the Terrain Pager. These will be used when deciding which Terrain
	 * tiles to load or unload.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Paging Settings")
	TArray<FAGX_TerrainPagingBodyReference> TrackedRigidBodies;
};
