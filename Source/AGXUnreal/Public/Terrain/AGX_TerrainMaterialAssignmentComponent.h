// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"

#include "AGX_TerrainMaterialAssignmentComponent.generated.h"

class UAGX_ShapeComponent;
class UAGX_ShapeMaterial;
class UAGX_TerrainMaterial;

USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_TerrainMaterialAssignmentData
{
	GENERATED_BODY()

	FAGX_TerrainMaterialAssignmentData()
	{
		InstanceTransforms.Add(FTransform::Identity);
	}

	UPROPERTY(VisibleAnywhere, Category = "AGX Terrain Material Assignment")
	FName ShapeComponentName;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Assignment")
	UAGX_TerrainMaterial* TerrainMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Assignment")
	UAGX_ShapeMaterial* ShapeMaterial = nullptr;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Assignment", AdvancedDisplay)
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
class AGXUNREAL_API UAGX_TerrainMaterialAssignmentComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UAGX_TerrainMaterialAssignmentComponent();

	/**
	 * Terrain Material Assignment data that is used to assign Terrain Materials to patches of the
	 * Terrain overlapped by the Shapes.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Assignment")
	TArray<FAGX_TerrainMaterialAssignmentData> TerrainMaterialAssignments;

	TArray<FAGX_TerrainMaterialAssignmentData>& GetTerrainMaterialAssignments();

	const TArray<FAGX_TerrainMaterialAssignmentData>& GetTerrainMaterialAssignments() const;

	void UpdateTerrainMaterialAssignments();

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

	void PrepareShapeForTerrainMaterialAssignment(UAGX_ShapeComponent& ShapeComponent);

	void RestoreShape(UAGX_ShapeComponent& ShapeComponent);
};
