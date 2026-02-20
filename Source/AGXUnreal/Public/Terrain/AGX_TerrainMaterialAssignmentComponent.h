// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Terrain/AGX_TerrainEnums.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"

#include "AGX_TerrainMaterialAssignmentComponent.generated.h"

USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_TerrainMaterialAssignmentData
{
	GENERATED_BODY()
};

UCLASS(
	ClassGroup = "AGX_Terrain", Category = "AGX", Meta = (BlueprintSpawnableComponent),
	HideCategories = (Cooking, LOD, Replication))
class AGXUNREAL_API UAGX_TerrainMaterialAssignmentComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UAGX_TerrainMaterialAssignmentComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Terrain Material Assignment")
	TArray<FAGX_TerrainMaterialAssignmentData> TerrainMaterialAssignments;

	// ~Begin UActorComponent interface.
	virtual void OnRegister() override;
	// ~End UActorComponent interface.

#if WITH_EDITOR
	// ~Begin USceneComponent interface.
	virtual void OnChildAttached(USceneComponent* Child) override;
	// ~End USceneComponent interface.
#endif

private:
	void ExcludeShapeFromSimulation(USceneComponent* Component);
};
