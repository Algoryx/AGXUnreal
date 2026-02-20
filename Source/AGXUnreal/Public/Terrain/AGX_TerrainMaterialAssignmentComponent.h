// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"

#include "AGX_TerrainMaterialAssignmentComponent.generated.h"

class UAGX_ShapeComponent;
class UAGX_TerrainMaterial;

USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_TerrainMaterialAssignmentData
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, Category = "AGX Terrain Material Assignment")
	FName ShapeComponentName;

	UPROPERTY()
	UAGX_ShapeComponent* ShapeComponent = nullptr;

	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Assignment")
	UAGX_TerrainMaterial* TerrainMaterial = nullptr;
};

UCLASS(
	ClassGroup = "AGX_Terrain", Category = "AGX", Meta = (BlueprintSpawnableComponent),
	HideCategories = (Cooking, LOD, Replication))
class AGXUNREAL_API UAGX_TerrainMaterialAssignmentComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UAGX_TerrainMaterialAssignmentComponent();

	TArray<FAGX_TerrainMaterialAssignmentData>& GetTerrainMaterialAssignments();

	const TArray<FAGX_TerrainMaterialAssignmentData>& GetTerrainMaterialAssignments() const;

	void UpdateTerrainMaterialAssignments();

	// ~Begin UActorComponent interface.
	virtual void OnRegister() override;
	// ~End UActorComponent interface.

#if WITH_EDITOR
	// ~Begin USceneComponent interface.
	virtual void OnChildAttached(USceneComponent* Child) override;
	virtual void OnChildDetached(USceneComponent* Child) override;
	// ~End USceneComponent interface.
#endif

private:
	void ExcludeShapeFromSimulation(USceneComponent* Component);

	UPROPERTY()
	TArray<FAGX_TerrainMaterialAssignmentData> TerrainMaterialAssignments;
};
