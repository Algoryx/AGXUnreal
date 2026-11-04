// Copyright 2026, Algoryx Simulation AB.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_TerrainRVTEditorLibrary.generated.h"

class ALandscape;
class UMaterialInterface;
class UMaterial;
class UMaterialExpression;

/** Editor-only authoring helpers for the Terrain RVT experiment. */
UCLASS()
class AGXUNREALEDITOR_API UAGX_TerrainRVTEditorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/** Creates a 2016 m square, non-partitioned Landscape with a flat and a sloped area. */
	UFUNCTION(BlueprintCallable, Category = "AGX Terrain RVT Experiment")
	static ALandscape* CreateExperimentLandscape(
		UMaterialInterface* Material, bool bSingleNaniteMesh = false);

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain RVT Experiment")
	static void BuildNanite(ALandscape* Landscape);

	/** Python's material-property enum omits the Displacement pin in UE 5.6. */
	UFUNCTION(BlueprintCallable, Category = "AGX Terrain RVT Experiment")
	static void ConnectNaniteDisplacement(UMaterial* Material, UMaterialExpression* Expression);
};
