// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyReference.h"
#include "AGX_Real.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_TerrainPagingBodyReference.generated.h"

USTRUCT()
struct AGXUNREAL_API FAGX_TerrainPagingBodyReference
{
	GENERATED_BODY()

	/**
	 * Rigid Body tracked by the Terrain Pager. Is is used when deciding which Terrain tiles
	 * to load or unload.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Paging Body")
	FAGX_RigidBodyReference RigidBody;

	/**
	 * The max distance from the Rigid Body at which new Terrain Tiles is guaranteed to be loaded
	 * [cm].
	 * Only relevant when using Terrain Paging.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Paging Body")
	FAGX_Real RequiredRadius {600.f};

	/**
	 * The max distance from the Rigid Body at which new Terrain Tiles will be preloaded [cm].
	 * Only relevant when using Terrain Paging.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Paging Body")
	FAGX_Real PreloadRadius {1000.f};
};
