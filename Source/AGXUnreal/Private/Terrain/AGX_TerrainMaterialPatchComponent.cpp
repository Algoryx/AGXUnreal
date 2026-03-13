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
		UAGX_TerrainMaterial* TerrainMaterial, UWorld* World)
	{
		if (TerrainMaterial == nullptr || World == nullptr)
			return nullptr;

		auto TerrainMaterialInstance = TerrainMaterial->GetOrCreateInstance(World);
		if (TerrainMaterialInstance == nullptr)
			return nullptr;

		return TerrainMaterialInstance->GetOrCreateTerrainMaterialNative(World);
	}

	FShapeBarrier* GetShapeBarrier(UAGX_ShapeComponent* ShapeComponent)
	{
		if (ShapeComponent == nullptr)
			return nullptr;

		return ShapeComponent->GetOrCreateNative();
	}

	FShapeMaterialBarrier* GetShapeMaterialBarrier(UAGX_ShapeMaterial* ShapeMaterial, UWorld* World)
	{
		if (ShapeMaterial == nullptr || World == nullptr)
			return nullptr;

		auto ShapeMaterialInstance = ShapeMaterial->GetOrCreateInstance(World);
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
	using namespace AGX_TerrainMaterialPatchComponent_helpers;
	TSet<FName> CurrentShapeNames;
	const auto AttachedShapes = GetAttachedShapes();
	for (const auto AttachedShape : AttachedShapes)
	{
		PrepareShapeForTerrainMaterialPatch(*AttachedShape);
		CurrentShapeNames.Add(GetShapeComponentName(*AttachedShape));
		AddAssignmentDataIfMissing(*AttachedShape);
	}

	// Remove patch data for Shapes no longer attached.
	TerrainMaterialPatches.RemoveAll(
		[&CurrentShapeNames](const FAGX_TerrainMaterialPatchData& Assignment)
		{
			return Assignment.ShapeComponentName.IsNone() ||
				   !CurrentShapeNames.Contains(Assignment.ShapeComponentName);
		});
}

bool UAGX_TerrainMaterialPatchComponent::AddPatchShapeInstance(
	FName ShapeName, const FAGX_Placement& Placement)
{
	if (ShapeName.IsNone())
		return false;

	FAGX_TerrainMaterialPatchData* PatchData = TerrainMaterialPatches.FindByPredicate(
		[ShapeName](const FAGX_TerrainMaterialPatchData& Data)
		{ return Data.ShapeComponentName == ShapeName; });
	if (PatchData == nullptr)
		return false;

	PatchData->InstancePlacements.Add(Placement);

	if (GetWorld() == nullptr || !GetWorld()->IsGameWorld())
		return false;

	FTerrainBarrier* TerrainBarrier =
		AGX_TerrainMaterialPatchComponent_helpers::GetTerrainBarrier(*this);
	if (TerrainBarrier == nullptr)
		return false;

	FAGX_TerrainMaterialPatchData SingleInstancePatch = *PatchData;
	SingleInstancePatch.InstancePlacements.Reset(1);
	SingleInstancePatch.InstancePlacements.Add(Placement);
	ApplyTerrainMaterialPatch(SingleInstancePatch, *TerrainBarrier);
	return true;
}

void UAGX_TerrainMaterialPatchComponent::AddPatch(
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
			TEXT("AddPatch called on Terrain Material Patch Component '%s' in '%s'. Unable to "
				 "find a Terrain parent, doing nothing."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return;
	}

	TArray<FAGX_Placement> Placement {FAGX_Placement()};
	ApplyTerrainMaterialPatch(
		ShapeComponent, TerrainMaterial, ShapeMaterial, Placement, *TerrainBarrier);
}

#if WITH_EDITOR
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
#endif // WITH_EDITOR

TArray<UAGX_ShapeComponent*> UAGX_TerrainMaterialPatchComponent::GetAttachedShapes() const
{
	TArray<UAGX_ShapeComponent*> Shapes;
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
					Shapes.Add(ShapeComponent);
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
				Shapes.Add(ShapeComponent);
		}
	}

	return Shapes;
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

	if (GIsReconstructingBlueprintInstances)
		return;

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
	using namespace AGX_TerrainMaterialPatchComponent_helpers;

	UAGX_ShapeComponent* Shape = GetAttachedShapeByName(*this, PatchData.ShapeComponentName);
	ApplyTerrainMaterialPatch(
		Shape, PatchData.TerrainMaterial, PatchData.ShapeMaterial, PatchData.InstancePlacements,
		TerrainBarrier);
}

void UAGX_TerrainMaterialPatchComponent::ApplyTerrainMaterialPatch(
	UAGX_ShapeComponent* Shape, UAGX_TerrainMaterial* TerrainMaterial,
	UAGX_ShapeMaterial* ShapeMaterial, const TArray<FAGX_Placement>& Placements,
	FTerrainBarrier& TerrainBarrier)
{
	using namespace AGX_TerrainMaterialPatchComponent_helpers;
	if (!bEnabled)
		return;

	if (Shape == nullptr || TerrainMaterial == nullptr)
		return;

	FTerrainMaterialBarrier* TerrainMaterialBarrier =
		GetTerrainMaterialBarrier(TerrainMaterial, GetWorld());
	const bool bShapeHadNative = Shape->HasNative();
	FShapeBarrier* ShapeBarrier = GetShapeBarrier(Shape);
	if (TerrainMaterialBarrier == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Terrain Material Patch Component '%s' in '%s', unable to create Terrain Material "
				 "Barrier from Terrain Material '%s'."),
			*GetName(), *GetLabelSafe(GetOwner()), *TerrainMaterial->GetName());
		return;
	}
	if (ShapeBarrier == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Terrain Material Patch Component '%s' in '%s', unable to create Shape "
				 "Barrier from Shape Component '%s'."),
			*GetName(), *GetLabelSafe(GetOwner()), *Shape->GetName());
		return;
	}

	FShapeMaterialBarrier* ShapeMaterialBarrier =
		GetShapeMaterialBarrier(ShapeMaterial, GetWorld());

	const FTransform OriginalRelativeTransform = Shape->GetRelativeTransform();
	for (const FAGX_Placement& Placement : Placements)
	{
		const FTransform Transform = Placement.ToTransform();

		// Instance transforms are interpreted relative to the shape's original transform.
		Shape->SetRelativeTransform(OriginalRelativeTransform * Transform);
		Shape->UpdateComponentToWorld();

		const int32 NumVoxels =
			TerrainBarrier.SetTerrainMaterial(*TerrainMaterialBarrier, *ShapeBarrier);
		if (NumVoxels == 0)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("ApplyTerrainMaterialPatch called on Terrain Material Patch Component '%s' in "
					 "'%s' but no voxels were overlapped or assigned when using Shape '%s' and "
					 "Terrain Material '%s'."),
				*GetName(), *GetLabelSafe(GetOwner()), *Shape->GetName(),
				*TerrainMaterial->GetName());
		}

		if (bLogPatchAssignments)
		{
			UE_LOG(
				LogAGX, Log,
				TEXT("Terrain Material Patch Component '%s' in '%s' assigned %d voxels when using "
					 "Shape '%s' and Terrain Material '%s'."),
				*GetName(), *GetLabelSafe(GetOwner()), NumVoxels, *Shape->GetName(),
				*TerrainMaterial->GetName());
		}

		if (ShapeMaterialBarrier != nullptr)
		{
			if (!TerrainBarrier.SetAssociatedMaterial(
					*TerrainMaterialBarrier, *ShapeMaterialBarrier))
			{
				UE_LOG(
					LogAGX, Log,
					TEXT("Terrain Material Patch Component '%s' in '%s' associating Shape Material "
						 "'%s' with Terrain Material '%s' failed. The Output Log may contain more "
						 "details."),
					*GetName(), *GetLabelSafe(GetOwner()), *ShapeMaterial->GetName(),
					*TerrainMaterial->GetName());
			}
		}
	}

	// Finally, we restore the original Shapes Transform since we moved it during "stamping"
	// above. We also release its Native if we created it. This is since the Shape may have the
	// bIncludeInSimulation set to false, in which case it will crash on Blueprint Reconstruction
	// since no one (Simulation) is keeping it alive.
	Shape->SetRelativeTransform(OriginalRelativeTransform);
	Shape->UpdateComponentToWorld();
	if (!bShapeHadNative && Shape->HasNative())
		Shape->ReleaseNative();
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
		RestoreShapeFromTerrainMaterialPatch(*ShapeComponent);
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

void UAGX_TerrainMaterialPatchComponent::RestoreShapeFromTerrainMaterialPatch(
	UAGX_ShapeComponent& ShapeComponent)
{
	ShapeComponent.bIncludeInSimulation = true;
	ShapeComponent.SetHiddenInGame(false);
	UAGX_ShapeComponent::RemoveSensorMaterial(ShapeComponent);
}
