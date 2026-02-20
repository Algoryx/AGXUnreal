// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainMaterialAssignmentComponent.h"

// AGX Dynamics for Unreal includes.
#include "Shapes/AGX_ShapeComponent.h"
#if WITH_EDITOR
#include "Utilities/AGX_BlueprintUtilities.h"
#endif

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
	TSet<FName> CurrentShapeNames;

	for (USceneComponent* Child : GetAttachChildren())
	{
		if (UAGX_ShapeComponent* ShapeComponent = Cast<UAGX_ShapeComponent>(Child))
		{
			ExcludeShapeFromSimulation(ShapeComponent);
			CurrentShapeNames.Add(GetShapeComponentName(*ShapeComponent));
			AddAssignmentDataIfMissing(*ShapeComponent);
		}
	}

#if WITH_EDITOR
	if (UBlueprint* Blueprint = FAGX_BlueprintUtilities::GetBlueprintFrom(*this))
	{
		for (UAGX_ShapeComponent* ShapeComponent :
			 FAGX_BlueprintUtilities::GetTemplateComponents<UAGX_ShapeComponent>(
				 *Blueprint, EAGX_Inherited::Include))
		{
			UActorComponent* Parent =
				FAGX_BlueprintUtilities::GetTemplateComponentAttachParent(ShapeComponent);
			if (Parent == this)
			{
				ExcludeShapeFromSimulation(ShapeComponent);
				CurrentShapeNames.Add(GetShapeComponentName(*ShapeComponent));
				AddAssignmentDataIfMissing(*ShapeComponent);
			}
		}
	}
#endif

	TerrainMaterialAssignments.RemoveAll(
		[&CurrentShapeNames](const FAGX_TerrainMaterialAssignmentData& Assignment)
		{
			return Assignment.ShapeComponentName.IsNone() ||
				   !CurrentShapeNames.Contains(Assignment.ShapeComponentName);
		});
}

FName UAGX_TerrainMaterialAssignmentComponent::GetShapeComponentName(
	const UAGX_ShapeComponent& ShapeComponent) const
{
#if WITH_EDITOR
	return FName(*FAGX_BlueprintUtilities::GetRegularNameFromTemplateComponentName(
		ShapeComponent.GetName()));
#else
	return ShapeComponent.GetFName();
#endif
}

void UAGX_TerrainMaterialAssignmentComponent::AddAssignmentDataIfMissing(
	const UAGX_ShapeComponent& ShapeComponent)
{
	const FName ShapeName = GetShapeComponentName(ShapeComponent);
	if (ShapeName.IsNone())
	{
		return;
	}

	if (TerrainMaterialAssignments.FindByPredicate(
			[ShapeName](const FAGX_TerrainMaterialAssignmentData& Assignment)
			{ return Assignment.ShapeComponentName == ShapeName; }) != nullptr)
	{
		return;
	}

	FAGX_TerrainMaterialAssignmentData& NewAssignment = TerrainMaterialAssignments.AddDefaulted_GetRef();
	NewAssignment.ShapeComponentName = ShapeName;
}

void UAGX_TerrainMaterialAssignmentComponent::RemoveAssignmentDataIfPresent(
	const UAGX_ShapeComponent& ShapeComponent)
{
	const FName ShapeName = GetShapeComponentName(ShapeComponent);
	if (ShapeName.IsNone())
	{
		return;
	}

	TerrainMaterialAssignments.RemoveAll(
		[ShapeName](const FAGX_TerrainMaterialAssignmentData& Assignment)
		{ return Assignment.ShapeComponentName == ShapeName; });
}

#if WITH_EDITOR
void UAGX_TerrainMaterialAssignmentComponent::OnChildAttached(USceneComponent* Child)
{
	Super::OnChildAttached(Child);
	ExcludeShapeFromSimulation(Child);
	if (UAGX_ShapeComponent* ShapeComponent = Cast<UAGX_ShapeComponent>(Child))
	{
		AddAssignmentDataIfMissing(*ShapeComponent);
	}
}

void UAGX_TerrainMaterialAssignmentComponent::OnChildDetached(USceneComponent* Child)
{
	Super::OnChildDetached(Child);
	if (UAGX_ShapeComponent* ShapeComponent = Cast<UAGX_ShapeComponent>(Child))
	{
		RemoveAssignmentDataIfPresent(*ShapeComponent);
	}
}
#endif

void UAGX_TerrainMaterialAssignmentComponent::ExcludeShapeFromSimulation(USceneComponent* Component)
{
	if (UAGX_ShapeComponent* Shape = Cast<UAGX_ShapeComponent>(Component))
	{
		Shape->bIncludeInSimulation = false;
	}
}
