// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainMaterialAssignmentComponent.h"

// AGX Dynamics for Unreal includes.
#include "Shapes/AGX_ShapeComponent.h"

UAGX_TerrainMaterialAssignmentComponent::UAGX_TerrainMaterialAssignmentComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAGX_TerrainMaterialAssignmentComponent::OnRegister()
{
	Super::OnRegister();

	for (USceneComponent* Child : GetAttachChildren())
		ExcludeShapeFromSimulation(Child);
}

#if WITH_EDITOR
void UAGX_TerrainMaterialAssignmentComponent::OnChildAttached(USceneComponent* Child)
{
	Super::OnChildAttached(Child);
	ExcludeShapeFromSimulation(Child);
}
#endif

void UAGX_TerrainMaterialAssignmentComponent::ExcludeShapeFromSimulation(USceneComponent* Component)
{
	if (UAGX_ShapeComponent* Shape = Cast<UAGX_ShapeComponent>(Component))
	{
		Shape->bIncludeInSimulation = false;
	}
}
