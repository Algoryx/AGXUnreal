// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_LandscapeMaterialLibrary.generated.h"

class UMaterial;
class UMaterialFunction;
class UMaterialExpression;
class ALandscapeProxy;

/** Editor scripting support for authoring Landscape deformation materials. */
UCLASS()
class AGXUNREALEDITOR_API UAGX_LandscapeMaterialLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Connect the Nanite Displacement pin, which UE 5.6's Python material enum does not expose.
	 * Magnitude is the full displacement span [mesh-local units], centered on Center in normalized
	 * input units. ClearLegacyOffsets disconnects the released WPO/PDO geometry offsets.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Landscape Material")
	static bool SetNaniteDisplacement(
		UMaterial* Material, UMaterialExpression* Expression, int32 OutputIndex = 0,
		float Magnitude = 1000.f, float Center = 0.5f, bool bClearLegacyOffsets = false);

	UFUNCTION(BlueprintPure, Category = "AGX Landscape Material")
	static TArray<UMaterialExpression*> GetMaterialExpressions(UMaterial* Material);

	/** Snapshot the graph before editing, preserving function interface identifiers on reruns. */
	UFUNCTION(BlueprintPure, Category = "AGX Landscape Material")
	static TArray<UMaterialExpression*> GetMaterialFunctionExpressions(UMaterialFunction* Function);

	/** Replace the consumers of an expression while adapting an existing material graph. */
	UFUNCTION(BlueprintCallable, Category = "AGX Landscape Material")
	static bool ReplaceExpressionConsumers(
		UMaterial* Material, UMaterialExpression* Previous, UMaterialExpression* Replacement,
		int32 OutputIndex = 0);

	/** Build vertical Nanite displacement data and retain original shading normals in UV6.
	 * IncludeUnloadedProxies processes a World Partition Landscape one proxy at a time.
	 * SavePackages saves the generated mesh and affected proxy packages.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Landscape Material")
	static bool BuildLandscapeNanite(
		ALandscapeProxy* Landscape, bool bIncludeUnloadedProxies = false,
		bool bSavePackages = false);

	/** Remove disconnected expressions after replacing the legacy deformation graph. */
	UFUNCTION(BlueprintCallable, Category = "AGX Landscape Material")
	static int32 RemoveUnusedExpressions(UMaterial* Material);
};
