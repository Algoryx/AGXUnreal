// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"

#include "AGX_TerrainMaterialPatchComponent.generated.h"

class FTerrainBarrier;
class UAGX_ShapeComponent;
class UAGX_ShapeMaterial;
class UAGX_TerrainMaterial;

USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_TerrainMaterialPatchData
{
	GENERATED_BODY()

	FAGX_TerrainMaterialPatchData()
	{
		InstanceTransforms.Add(FTransform::Identity);
	}

	UPROPERTY(VisibleAnywhere, Category = "AGX Terrain Material Patch")
	FName ShapeComponentName;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Patch")
	UAGX_TerrainMaterial* TerrainMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Patch")
	UAGX_ShapeMaterial* ShapeMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Patch", AdvancedDisplay)
	TArray<FTransform> InstanceTransforms;
};

/**
 * Component that makes it possible to assign specific Terrain Materials to different patches of a
 * Terrain using AGX Shape Components. Add AGX Shape Components as children to this Component to use
 * them to select overlapping patches where specific Terrain Materials will be assigned.
 *
 * Also, it is possible to associate each Terrain Material with a Shape Material that will be active
 * whenever a contact is created on a patch with a specific Terrain Material.
 * This way, the surface properties of the Terrain can also be set differently for different
 * patches of the Terrain.
 */
UCLASS(
	ClassGroup = "AGX_Terrain", Category = "AGX", Meta = (BlueprintSpawnableComponent),
	HideCategories = (Cooking, LOD, Replication))
class AGXUNREAL_API UAGX_TerrainMaterialPatchComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UAGX_TerrainMaterialPatchComponent();

	/**
	 * Terrain Material Patch data that is used to assign Terrain Materials to patches of the
	 * Terrain overlapped by the Shapes.
	 * This data is applied at BeginPlay and changes to it durint Play will generally not havee any
	 * effect. One exception is when adding instances through the AddShapeInstance function.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Patch")
	TArray<FAGX_TerrainMaterialPatchData> TerrainMaterialPatches;

	TArray<FAGX_TerrainMaterialPatchData>& GetTerrainMaterialPatches();

	const TArray<FAGX_TerrainMaterialPatchData>& GetTerrainMaterialPatches() const;

	void UpdateTerrainMaterialPatches();

	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Patch")
	void AddShapeInstance(FName ShapeName, FTransform InstanceTransform);

	// ~Begin UObject interface.
	virtual bool CanEditChange(const FProperty* InProperty) const override;
	// ~End UObject interface.

	// ~Begin UActorComponent interface.
	virtual void BeginPlay() override;
	// ~End UActorComponent interface.

#if WITH_EDITOR
	// ~Begin USceneComponent interface.
	virtual void OnChildAttached(USceneComponent* Child) override;
	virtual void OnChildDetached(USceneComponent* Child) override;
	// ~End USceneComponent interface.
#endif

private:
	void AddAssignmentDataIfMissing(const UAGX_ShapeComponent& ShapeComponent);

	void RemoveAssignmentDataIfPresent(const UAGX_ShapeComponent& ShapeComponent);

	void PrepareShapeForTerrainMaterialPatch(UAGX_ShapeComponent& ShapeComponent);

	void RestoreShape(UAGX_ShapeComponent& ShapeComponent);

	void ApplyTerrainMaterialPatch(
		const FAGX_TerrainMaterialPatchData& PatchData, FTerrainBarrier& TerrainBarrier);
};
