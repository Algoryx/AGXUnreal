// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainMaterialPatchComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_PropertyChangedDispatcher.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Materials/AGX_TerrainMaterial.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Terrain/AGX_MovableTerrainComponent.h"
#include "Terrain/AGX_Terrain.h"
#include "Terrain/TerrainBarrier.h"
#if WITH_EDITOR
#include "Utilities/AGX_BlueprintUtilities.h"
#endif

namespace AGX_TerrainMaterialPatchComponent_helpers
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

	FTerrainBarrier* GetTerrainBarrier(UAGX_TerrainMaterialPatchComponent& Component)
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
		UAGX_TerrainMaterialPatchComponent& Component, const FName& ShapeName)
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

UAGX_TerrainMaterialPatchComponent::UAGX_TerrainMaterialPatchComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

TArray<FAGX_TerrainMaterialPatchData>&
UAGX_TerrainMaterialPatchComponent::GetTerrainMaterialPatches()
{
	return TerrainMaterialPatches;
}

const TArray<FAGX_TerrainMaterialPatchData>&
UAGX_TerrainMaterialPatchComponent::GetTerrainMaterialPatches() const
{
	return TerrainMaterialPatches;
}

void UAGX_TerrainMaterialPatchComponent::UpdateTerrainMaterialPatches()
{
	TSet<FName> CurrentShapeNames;

	if (IsInBlueprint())
	{
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
					PrepareShapeForTerrainMaterialPatch(*ShapeComponent);
					CurrentShapeNames.Add(
						AGX_TerrainMaterialPatchComponent_helpers::GetShapeComponentName(
							*ShapeComponent));
					AddAssignmentDataIfMissing(*ShapeComponent);
				}
			}
		}
#endif // WITH_EDITOR
	}
	else
	{
		for (USceneComponent* Child : GetAttachChildren())
		{
			if (UAGX_ShapeComponent* ShapeComponent = Cast<UAGX_ShapeComponent>(Child))
			{
				PrepareShapeForTerrainMaterialPatch(*ShapeComponent);
				CurrentShapeNames.Add(
					AGX_TerrainMaterialPatchComponent_helpers::GetShapeComponentName(
						*ShapeComponent));
				AddAssignmentDataIfMissing(*ShapeComponent);
			}
		}
	}

	TerrainMaterialPatches.RemoveAll(
		[&CurrentShapeNames](const FAGX_TerrainMaterialPatchData& Assignment)
		{
			return Assignment.ShapeComponentName.IsNone() ||
				   !CurrentShapeNames.Contains(Assignment.ShapeComponentName);
		});
}

bool UAGX_TerrainMaterialPatchComponent::CanEditChange(const FProperty* InProperty) const
{
	const bool SuperCanEditChange = Super::CanEditChange(InProperty);
	if (!SuperCanEditChange)
		return false;

	if (InProperty == nullptr)
		return SuperCanEditChange;

	const bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
	if (bIsPlaying)
	{
		// List of names of properties that does not support editing after initialization.
		static const TArray<FName> PropertiesNotEditableDuringPlay = {
			AGX_MEMBER_NAME(TerrainMaterialPatches)};

		if (PropertiesNotEditableDuringPlay.Contains(InProperty->GetFName()))
			return false;
	}

	return SuperCanEditChange;
}

void UAGX_TerrainMaterialPatchComponent::AddAssignmentDataIfMissing(
	const UAGX_ShapeComponent& ShapeComponent)
{
	const FName ShapeName =
		AGX_TerrainMaterialPatchComponent_helpers::GetShapeComponentName(ShapeComponent);
	if (ShapeName.IsNone())
	{
		return;
	}

	if (TerrainMaterialPatches.FindByPredicate(
			[ShapeName](const FAGX_TerrainMaterialPatchData& Assignment)
			{ return Assignment.ShapeComponentName == ShapeName; }) != nullptr)
	{
		return;
	}

	FAGX_TerrainMaterialPatchData& NewAssignment = TerrainMaterialPatches.AddDefaulted_GetRef();
	NewAssignment.ShapeComponentName = ShapeName;
}

void UAGX_TerrainMaterialPatchComponent::RemoveAssignmentDataIfPresent(
	const UAGX_ShapeComponent& ShapeComponent)
{
	const FName ShapeName =
		AGX_TerrainMaterialPatchComponent_helpers::GetShapeComponentName(ShapeComponent);
	if (ShapeName.IsNone())
	{
		return;
	}

	TerrainMaterialPatches.RemoveAll([ShapeName](const FAGX_TerrainMaterialPatchData& Assignment)
									 { return Assignment.ShapeComponentName == ShapeName; });
}

void UAGX_TerrainMaterialPatchComponent::BeginPlay()
{
	Super::BeginPlay();
	UpdateTerrainMaterialPatches();

	FTerrainBarrier* TerrainBarrier =
		AGX_TerrainMaterialPatchComponent_helpers::GetTerrainBarrier(*this);
	if (TerrainBarrier == nullptr)
		return;

	for (const FAGX_TerrainMaterialPatchData& AssignmentData : TerrainMaterialPatches)
	{
		if (AssignmentData.TerrainMaterial == nullptr)
			continue;

		UAGX_ShapeComponent* ShapeComponent =
			AGX_TerrainMaterialPatchComponent_helpers::GetAttachedShapeByName(
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

		if (AssignmentData.ShapeMaterial != nullptr)
		{
			auto ShapeMaterialInstance =
				AssignmentData.ShapeMaterial->GetOrCreateInstance(GetWorld());
			if (ShapeMaterialInstance == nullptr)
				continue;

			FShapeMaterialBarrier* ShapeMaterialBarrier =
				ShapeMaterialInstance->GetOrCreateShapeMaterialNative(GetWorld());
			if (ShapeMaterialBarrier == nullptr)
				continue;

			TerrainBarrier->SetAssociatedMaterial(*TerrainMaterialBarrier, *ShapeMaterialBarrier);
		}
	}
}

#if WITH_EDITOR
void UAGX_TerrainMaterialPatchComponent::OnChildAttached(USceneComponent* Child)
{
	Super::OnChildAttached(Child);
	if (UAGX_ShapeComponent* ShapeComponent = Cast<UAGX_ShapeComponent>(Child))
	{
		PrepareShapeForTerrainMaterialPatch(*ShapeComponent);
		AddAssignmentDataIfMissing(*ShapeComponent);
	}
}

void UAGX_TerrainMaterialPatchComponent::OnChildDetached(USceneComponent* Child)
{
	Super::OnChildDetached(Child);
	if (UAGX_ShapeComponent* ShapeComponent = Cast<UAGX_ShapeComponent>(Child))
	{
		RestoreShape(*ShapeComponent);
		RemoveAssignmentDataIfPresent(*ShapeComponent);
	}
}
#endif

void UAGX_TerrainMaterialPatchComponent::PrepareShapeForTerrainMaterialPatch(
	UAGX_ShapeComponent& ShapeComponent)
{
	ShapeComponent.bIncludeInSimulation = false;
	ShapeComponent.SetHiddenInGame(true);
	UAGX_ShapeComponent::ApplySensorMaterial(ShapeComponent);
}

void UAGX_TerrainMaterialPatchComponent::RestoreShape(UAGX_ShapeComponent& ShapeComponent)
{
	ShapeComponent.bIncludeInSimulation = true;
	ShapeComponent.SetHiddenInGame(false);
	UAGX_ShapeComponent::RemoveSensorMaterial(ShapeComponent);
}
