// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainMaterialAssignmentComponent.h"

// AGX Dynamics for Unreal includes.
#include "Materials/AGX_TerrainMaterial.h"
#include "Shapes/AGX_ShapeComponent.h"

UAGX_TerrainMaterialAssignmentComponent::UAGX_TerrainMaterialAssignmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

TArray<FAGX_TerrainMaterialAssignmentData>&
UAGX_TerrainMaterialAssignmentComponent::GetTerrainMaterialAssignments()
{
	return TerrainMaterialAssignments;
}

const TArray<FAGX_TerrainMaterialAssignmentData>&
UAGX_TerrainMaterialAssignmentComponent::GetTerrainMaterialAssignments() const
{
	return TerrainMaterialAssignments;
}

void UAGX_TerrainMaterialAssignmentComponent::UpdateTerrainMaterialAssignments()
{
	TMap<UAGX_ShapeComponent*, UAGX_TerrainMaterial*> ExistingMaterials;
	ExistingMaterials.Reserve(TerrainMaterialAssignments.Num());
	for (const FAGX_TerrainMaterialAssignmentData& Assignment : TerrainMaterialAssignments)
	{
		if (Assignment.ShapeComponent == nullptr)
		{
			continue;
		}

		ExistingMaterials.Add(Assignment.ShapeComponent, Assignment.TerrainMaterial);
	}

	TArray<FAGX_TerrainMaterialAssignmentData> UpdatedAssignments;
	UpdatedAssignments.Reserve(GetAttachChildren().Num());
	for (USceneComponent* Child : GetAttachChildren())
	{
		UAGX_ShapeComponent* ShapeComponent = Cast<UAGX_ShapeComponent>(Child);
		if (ShapeComponent == nullptr)
		{
			continue;
		}

		ExcludeShapeFromSimulation(ShapeComponent);

		FAGX_TerrainMaterialAssignmentData& NewAssignment = UpdatedAssignments.AddDefaulted_GetRef();
		NewAssignment.ShapeComponent = ShapeComponent;
		NewAssignment.ShapeComponentName = ShapeComponent->GetFName();
		if (UAGX_TerrainMaterial* const* ExistingMaterial = ExistingMaterials.Find(ShapeComponent))
		{
			NewAssignment.TerrainMaterial = *ExistingMaterial;
		}
	}

	TerrainMaterialAssignments = MoveTemp(UpdatedAssignments);
}

void UAGX_TerrainMaterialAssignmentComponent::OnRegister()
{
	Super::OnRegister();
	UpdateTerrainMaterialAssignments();
}

#if WITH_EDITOR
void UAGX_TerrainMaterialAssignmentComponent::OnChildAttached(USceneComponent* Child)
{
	Super::OnChildAttached(Child);
	ExcludeShapeFromSimulation(Child);
	UpdateTerrainMaterialAssignments();
}

void UAGX_TerrainMaterialAssignmentComponent::OnChildDetached(USceneComponent* Child)
{
	Super::OnChildDetached(Child);
	UpdateTerrainMaterialAssignments();
}
#endif

void UAGX_TerrainMaterialAssignmentComponent::ExcludeShapeFromSimulation(USceneComponent* Component)
{
	if (UAGX_ShapeComponent* Shape = Cast<UAGX_ShapeComponent>(Component))
	{
		Shape->bIncludeInSimulation = false;
	}
}
