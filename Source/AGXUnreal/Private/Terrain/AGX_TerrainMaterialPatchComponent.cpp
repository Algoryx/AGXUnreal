// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainMaterialPatchComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
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
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

namespace AGX_TerrainMaterialPatchComponent_helpers
{
	FName GetShapeComponentName(const UAGX_ShapeComponent& ShapeComponent)
	{
		if (ShapeComponent.IsInBlueprint())
		{
#if WITH_EDITOR
			return FName(*FAGX_BlueprintUtilities::GetRegularNameFromTemplateComponentName(
				ShapeComponent.GetName()));
#else
			return ShapeComponent.GetFName();
#endif
		}
		else
		{
			return ShapeComponent.GetFName();
		}
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

	FTerrainMaterialBarrier* GetTerrainMaterialBarrier(
		const FAGX_TerrainMaterialPatchData& PatchData, UWorld* World)
	{
		if (PatchData.TerrainMaterial == nullptr || World == nullptr)
			return nullptr;

		auto TerrainMaterialInstance = PatchData.TerrainMaterial->GetOrCreateInstance(World);
		if (TerrainMaterialInstance == nullptr)
			return nullptr;

		return TerrainMaterialInstance->GetOrCreateTerrainMaterialNative(World);
	}

	FShapeBarrier* GetShapeBarrier(
		UAGX_TerrainMaterialPatchComponent& Component,
		const FAGX_TerrainMaterialPatchData& PatchData, UAGX_ShapeComponent*& OutShapeComponent)
	{
		OutShapeComponent = GetAttachedShapeByName(Component, PatchData.ShapeComponentName);
		if (OutShapeComponent == nullptr)
			return nullptr;

		return OutShapeComponent->GetOrCreateNative();
	}

	FShapeMaterialBarrier* GetShapeMaterialBarrier(
		const FAGX_TerrainMaterialPatchData& PatchData, UWorld* World)
	{
		if (PatchData.ShapeMaterial == nullptr || World == nullptr)
			return nullptr;

		auto ShapeMaterialInstance = PatchData.ShapeMaterial->GetOrCreateInstance(World);
		if (ShapeMaterialInstance == nullptr)
			return nullptr;

		return ShapeMaterialInstance->GetOrCreateShapeMaterialNative(World);
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

void UAGX_TerrainMaterialPatchComponent::AddShapeInstance(
	FName ShapeName, FTransform InstanceTransform)
{
	if (ShapeName.IsNone())
		return;

	FAGX_TerrainMaterialPatchData* PatchData = TerrainMaterialPatches.FindByPredicate(
		[ShapeName](const FAGX_TerrainMaterialPatchData& Data)
		{ return Data.ShapeComponentName == ShapeName; });
	if (PatchData == nullptr)
		return;

	PatchData->InstanceTransforms.Add(InstanceTransform);

	if (GetWorld() != nullptr && GetWorld()->IsGameWorld())
	{
		FTerrainBarrier* TerrainBarrier =
			AGX_TerrainMaterialPatchComponent_helpers::GetTerrainBarrier(*this);
		if (TerrainBarrier == nullptr)
			return;

		FAGX_TerrainMaterialPatchData SingleInstancePatch = *PatchData;
		SingleInstancePatch.InstanceTransforms.Reset(1);
		SingleInstancePatch.InstanceTransforms.Add(InstanceTransform);
		ApplyTerrainMaterialPatch(SingleInstancePatch, *TerrainBarrier);
	}
}

void UAGX_TerrainMaterialPatchComponent::ApplyPatch(
	UAGX_ShapeComponent* ShapeComponent, UAGX_TerrainMaterial* TerrainMaterial,
	UAGX_ShapeMaterial* ShapeMaterial)
{
	if (ShapeComponent == nullptr || TerrainMaterial == nullptr)
		return;

	if (GetWorld() == nullptr || !GetWorld()->IsGameWorld())
		return;

	FTerrainBarrier* TerrainBarrier =
		AGX_TerrainMaterialPatchComponent_helpers::GetTerrainBarrier(*this);
	if (TerrainBarrier == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("ApplyPatch called on Terrain Material Patch Component '%s' in '%s'. Unable to "
				 "find a Terrain parent, doing nothing."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	FAGX_TerrainMaterialPatchData PatchData;
	PatchData.ShapeComponentName =
		AGX_TerrainMaterialPatchComponent_helpers::GetShapeComponentName(*ShapeComponent);
	PatchData.TerrainMaterial = TerrainMaterial;
	PatchData.ShapeMaterial = ShapeMaterial;

	ApplyTerrainMaterialPatch(PatchData, *TerrainBarrier);
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
	{
		const FString Message = FString::Printf(
			TEXT("AGX Terrain Material Patch '%s' in '%s' could not find an AGX Terrain or AGX "
				 "Movable Terrain parent. Ignoring terrain material patch assignments."),
			*GetName(), *GetLabelSafe(GetOwner()));
		FAGX_NotificationUtilities::ShowNotification(Message, SNotificationItem::CS_Fail);
		return;
	}

	for (const FAGX_TerrainMaterialPatchData& AssignmentData : TerrainMaterialPatches)
	{
		ApplyTerrainMaterialPatch(AssignmentData, *TerrainBarrier);
	}
}

void UAGX_TerrainMaterialPatchComponent::ApplyTerrainMaterialPatch(
	const FAGX_TerrainMaterialPatchData& PatchData, FTerrainBarrier& TerrainBarrier)
{
	FTerrainMaterialBarrier* TerrainMaterialBarrier =
		AGX_TerrainMaterialPatchComponent_helpers::GetTerrainMaterialBarrier(PatchData, GetWorld());
	UAGX_ShapeComponent* ShapeComponent = nullptr;
	FShapeBarrier* ShapeBarrier = AGX_TerrainMaterialPatchComponent_helpers::GetShapeBarrier(
		*this, PatchData, ShapeComponent);
	if (TerrainMaterialBarrier == nullptr || ShapeBarrier == nullptr)
		return;

	FShapeMaterialBarrier* ShapeMaterialBarrier =
		AGX_TerrainMaterialPatchComponent_helpers::GetShapeMaterialBarrier(PatchData, GetWorld());

	const FTransform OriginalRelativeTransform = ShapeComponent->GetRelativeTransform();
	for (const FTransform& InstanceTransform : PatchData.InstanceTransforms)
	{
		// Instance transforms are interpreted relative to the shape's original transform.
		ShapeComponent->SetRelativeTransform(OriginalRelativeTransform * InstanceTransform);
		ShapeComponent->UpdateComponentToWorld();

		TerrainBarrier.SetTerrainMaterial(*TerrainMaterialBarrier, *ShapeBarrier);
		if (ShapeMaterialBarrier != nullptr)
			TerrainBarrier.SetAssociatedMaterial(*TerrainMaterialBarrier, *ShapeMaterialBarrier);
	}

	// Finally, we restore the original Shapes Transform since we moved it during "stamping"
	// above.
	ShapeComponent->SetRelativeTransform(OriginalRelativeTransform);
	ShapeComponent->UpdateComponentToWorld();
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
