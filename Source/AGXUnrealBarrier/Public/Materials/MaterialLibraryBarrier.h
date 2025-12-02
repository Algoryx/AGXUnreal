// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Materials/ContactMaterialBarrier.h"
#include "Materials/ShapeMaterialBarrier.h"
#include "Materials/TerrainMaterialBarrier.h"

// Unreal Engine includes.
#include "Containers/Array.h"

namespace AGX_MaterialLibraryBarrier
{
	AGXUNREALBARRIER_API TArray<FString> GetAvailableLibraryContactMaterials();
	AGXUNREALBARRIER_API TOptional<FContactMaterialBarrier> LoadContactMaterialProfile(
		const FString& MaterialName);

	AGXUNREALBARRIER_API TArray<FString> GetAvailableLibraryShapeMaterials();
	AGXUNREALBARRIER_API TOptional<FShapeMaterialBarrier> LoadShapeMaterialProfile(
		const FString& MaterialName);

	AGXUNREALBARRIER_API TArray<FString> GetAvailableLibraryTerrainMaterials();
	AGXUNREALBARRIER_API TOptional<FTerrainMaterialBarrier> LoadTerrainMaterialProfile(
		const FString& MaterialName);
}
