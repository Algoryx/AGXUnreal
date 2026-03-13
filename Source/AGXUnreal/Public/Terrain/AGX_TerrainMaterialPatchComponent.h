// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Placement.h"

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
		InstancePlacements.AddDefaulted();
	}

	/** Name of the attached AGX Shape Component that defines this patch area. */
	UPROPERTY(VisibleAnywhere, Category = "AGX Terrain Material Patch")
	FName ShapeComponentName;

	/** Terrain Material that will be assigned to the Terrain voxels overlapped by the Shape. */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Patch")
	UAGX_TerrainMaterial* TerrainMaterial = nullptr;

	/** Optional Shape Material to associate with the selected Terrain Material. */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Patch")
	UAGX_ShapeMaterial* ShapeMaterial = nullptr;

	/** Per-shape local location/rotation values used to create one or more patch instances. */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Patch Advanced", AdvancedDisplay)
	TArray<FAGX_Placement> InstancePlacements;

	/**
	 * If set to true, the Shape instances are debug rendered.
	 * Debug rendering of Trimesh Shapes is currently not supported.
	 * This operation may be performance heavy.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Patch Advanced", AdvancedDisplay)
	bool bDebugRenderInstances {false};
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
	 * Whether or not this Component is enabled.
	 * If set to false, no patch assignments will occur.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Patch")
	bool bEnabled {true};

	/**
	 * Terrain Material Patch data that is used to assign Terrain Materials to patches of the
	 * Terrain overlapped by the Shapes.
	 *
	 * Elements of this data is automatically added or removed according to the AGX Shape Components
	 * attached to this Component.
	 *
	 * This data is applied at BeginPlay and changes to it durint Play will generally not have any
	 * effect. One exception is when adding instances through the AddShapeInstance function.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Patch")
	TArray<FAGX_TerrainMaterialPatchData> TerrainMaterialPatches;

	/**
	 * For each Terrain Material patch assignment, a log message is printed with the number of
	 * overlapping voxels for each assignment.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Patch", AdvancedDisplay)
	bool bLogPatchAssignments {false};

	TArray<FAGX_TerrainMaterialPatchData>& GetTerrainMaterialPatches();

	const TArray<FAGX_TerrainMaterialPatchData>& GetTerrainMaterialPatches() const;

	/**
	 * Force an update of the Terrain Material Patches given the current children (Shape)
	 * attachments.
	 */
	void UpdateTerrainMaterialPatches();

	/**
	 * Add a Shape instance of an existing patch.
	 * If the ShapeName does not match an
	 * existing Shape Component's name, this
	 * function has no effect.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Patch")
	void AddPatchShapeInstance(FName ShapeName, const FAGX_Placement& Placement);

	/**
	 * Apply a Terrain Material patch during Play.
	 * This function should only be called during Play and will not modify the
	 * TerrainMaterialPatches property, i.e. calling this function will not be reflected in the
	 * Details Panel.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Patch")
	void AddPatch(
		UAGX_ShapeComponent* ShapeComponent, UAGX_TerrainMaterial* TerrainMaterial,
		UAGX_ShapeMaterial* ShapeMaterial);

	/**
	 * Returns all attached AGX Shape Components.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Terrain Material Patch")
	TArray<UAGX_ShapeComponent*> GetAttachedShapes() const;

	// ~Begin UObject interface.
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
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

	void ApplyTerrainMaterialPatch(
		UAGX_ShapeComponent* Shape, UAGX_TerrainMaterial* TerrainMaterial,
		UAGX_ShapeMaterial* ShapeMaterial, const TArray<FAGX_Placement>& Placements,
		FTerrainBarrier& TerrainBarrier);
};
