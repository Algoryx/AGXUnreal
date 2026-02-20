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
	TMap<FName, FAGX_TerrainMaterialAssignmentData> ExistingAssignments;
	ExistingAssignments.Reserve(TerrainMaterialAssignments.Num());
	for (const FAGX_TerrainMaterialAssignmentData& Assignment : TerrainMaterialAssignments)
	{
		if (Assignment.ShapeComponentName.IsNone())
		{
			continue;
		}

		ExistingAssignments.Add(Assignment.ShapeComponentName, Assignment);
	}

	TArray<FAGX_TerrainMaterialAssignmentData> UpdatedAssignments;
	UpdatedAssignments.Reserve(GetAttachChildren().Num());
	TSet<FName> AddedShapeNames;
	AddedShapeNames.Reserve(GetAttachChildren().Num());

	auto AddShapeAssignment = [&](UAGX_ShapeComponent* ShapeComponent)
	{
		if (ShapeComponent == nullptr)
		{
			return;
		}

		ExcludeShapeFromSimulation(ShapeComponent);

		const FName ShapeName = [&]()
		{
#if WITH_EDITOR
			return FName(*FAGX_BlueprintUtilities::GetRegularNameFromTemplateComponentName(
				ShapeComponent->GetName()));
#else
			return ShapeComponent->GetFName();
#endif
		}();
		if (ShapeName.IsNone() || AddedShapeNames.Contains(ShapeName))
		{
			return;
		}
		AddedShapeNames.Add(ShapeName);

		FAGX_TerrainMaterialAssignmentData& NewAssignment = UpdatedAssignments.AddDefaulted_GetRef();
		NewAssignment.ShapeComponentName = ShapeName;
		if (const FAGX_TerrainMaterialAssignmentData* ExistingAssignment =
				ExistingAssignments.Find(ShapeName))
		{
			NewAssignment.TerrainMaterial = ExistingAssignment->TerrainMaterial;
			NewAssignment.ShapeMaterial = ExistingAssignment->ShapeMaterial;
		}
	};

	for (USceneComponent* Child : GetAttachChildren())
	{
		AddShapeAssignment(Cast<UAGX_ShapeComponent>(Child));
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
				AddShapeAssignment(ShapeComponent);
			}
		}
	}
#endif

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
