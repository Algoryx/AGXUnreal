// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainMaterialAssignmentComponent.h"

// AGX Dynamics for Unreal includes.
#include "Materials/AGX_TerrainMaterial.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Terrain/AGX_MovableTerrainComponent.h"
#include "Terrain/AGX_Terrain.h"
#include "Terrain/TerrainBarrier.h"
#if WITH_EDITOR
#include "Utilities/AGX_BlueprintUtilities.h"
#endif

namespace AGX_TerrainMaterialAssignmentComponent_helpers
{
	FName GetShapeComponentName(const UAGX_ShapeComponent& ShapeComponent)
	{
#if WITH_EDITOR
		return FName(*FAGX_BlueprintUtilities::GetRegularNameFromTemplateComponentName(
			ShapeComponent.GetName()));
#else
		return ShapeComponent.GetFName();
#endif
	}

	FTerrainBarrier* GetTerrainBarrier(UAGX_TerrainMaterialAssignmentComponent& Component)
	{
		if (AAGX_Terrain* Terrain = Cast<AAGX_Terrain>(Component.GetOwner()))
		{
			return Terrain->GetOrCreateNative();
		}

		if (USceneComponent* Parent = Component.GetAttachParent())
		{
			if (UAGX_MovableTerrainComponent* MovableTerrain =
					Cast<UAGX_MovableTerrainComponent>(Parent))
			{
				return MovableTerrain->GetOrCreateNative();
			}
		}

		return nullptr;
	}

	UAGX_ShapeComponent* GetAttachedShapeByName(
		UAGX_TerrainMaterialAssignmentComponent& Component, const FName& ShapeName)
	{
		if (ShapeName.IsNone())
		{
			return nullptr;
		}

		for (USceneComponent* Child : Component.GetAttachChildren())
		{
			if (UAGX_ShapeComponent* ShapeComponent = Cast<UAGX_ShapeComponent>(Child))
			{
				if (GetShapeComponentName(*ShapeComponent) == ShapeName)
				{
					return ShapeComponent;
				}
			}
		}

		return nullptr;
	}
}

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
			PrepareShapeForTerrainMaterialAssignment(*ShapeComponent);
			CurrentShapeNames.Add(
				AGX_TerrainMaterialAssignmentComponent_helpers::GetShapeComponentName(
					*ShapeComponent));
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
				PrepareShapeForTerrainMaterialAssignment(*ShapeComponent);
				CurrentShapeNames.Add(
					AGX_TerrainMaterialAssignmentComponent_helpers::GetShapeComponentName(
						*ShapeComponent));
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

void UAGX_TerrainMaterialAssignmentComponent::AddAssignmentDataIfMissing(
	const UAGX_ShapeComponent& ShapeComponent)
{
	const FName ShapeName =
		AGX_TerrainMaterialAssignmentComponent_helpers::GetShapeComponentName(ShapeComponent);
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
	const FName ShapeName =
		AGX_TerrainMaterialAssignmentComponent_helpers::GetShapeComponentName(ShapeComponent);
	if (ShapeName.IsNone())
	{
		return;
	}

	TerrainMaterialAssignments.RemoveAll(
		[ShapeName](const FAGX_TerrainMaterialAssignmentData& Assignment)
		{ return Assignment.ShapeComponentName == ShapeName; });
}

void UAGX_TerrainMaterialAssignmentComponent::BeginPlay()
{
	Super::BeginPlay();
	UpdateTerrainMaterialAssignments();

	FTerrainBarrier* TerrainBarrier =
		AGX_TerrainMaterialAssignmentComponent_helpers::GetTerrainBarrier(*this);
	if (TerrainBarrier == nullptr)
		return;

	for (const FAGX_TerrainMaterialAssignmentData& AssignmentData : TerrainMaterialAssignments)
	{
		if (AssignmentData.TerrainMaterial == nullptr)
			continue;

		UAGX_ShapeComponent* ShapeComponent =
			AGX_TerrainMaterialAssignmentComponent_helpers::GetAttachedShapeByName(
				*this, AssignmentData.ShapeComponentName);
		if (ShapeComponent == nullptr)
			continue;

		auto TerrainMaterialInstance =
			AssignmentData.TerrainMaterial->GetOrCreateInstance(GetWorld());
		if (TerrainMaterialInstance == nullptr)
			continue;

		FTerrainMaterialBarrier* TerrainMaterialBarrier =
			TerrainMaterialInstance->GetOrCreateTerrainMaterialNative(GetWorld());
		FShapeBarrier* ShapeBarrier = ShapeComponent->GetOrCreateNative();
		if (TerrainMaterialBarrier == nullptr || ShapeBarrier == nullptr)
			continue;

		TerrainBarrier->SetTerrainMaterial(*TerrainMaterialBarrier, *ShapeBarrier);
	}
}

#if WITH_EDITOR
void UAGX_TerrainMaterialAssignmentComponent::OnChildAttached(USceneComponent* Child)
{
	Super::OnChildAttached(Child);
	if (UAGX_ShapeComponent* ShapeComponent = Cast<UAGX_ShapeComponent>(Child))
	{
		PrepareShapeForTerrainMaterialAssignment(*ShapeComponent);
		AddAssignmentDataIfMissing(*ShapeComponent);
	}
}

void UAGX_TerrainMaterialAssignmentComponent::OnChildDetached(USceneComponent* Child)
{
	Super::OnChildDetached(Child);
	if (UAGX_ShapeComponent* ShapeComponent = Cast<UAGX_ShapeComponent>(Child))
	{
		RestoreShape(*ShapeComponent);
		RemoveAssignmentDataIfPresent(*ShapeComponent);
	}
}
#endif

void UAGX_TerrainMaterialAssignmentComponent::PrepareShapeForTerrainMaterialAssignment(
	UAGX_ShapeComponent& ShapeComponent)
{
	ShapeComponent.bIncludeInSimulation = false;
	ShapeComponent.SetHiddenInGame(true);
	UAGX_ShapeComponent::ApplySensorMaterial(ShapeComponent);
}

void UAGX_TerrainMaterialAssignmentComponent::RestoreShape(UAGX_ShapeComponent& ShapeComponent)
{
	ShapeComponent.bIncludeInSimulation = true;
	ShapeComponent.SetHiddenInGame(false);
	UAGX_ShapeComponent::RemoveSensorMaterial(ShapeComponent);
}
