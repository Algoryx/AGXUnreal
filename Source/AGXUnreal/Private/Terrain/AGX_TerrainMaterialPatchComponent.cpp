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
#include "Terrain/TerrainPagerBarrier.h"
#if WITH_EDITOR
#include "Utilities/AGX_BlueprintUtilities.h"
#endif
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "Misc/Optional.h"

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

		for (UAGX_ShapeComponent* ShapeComponent : Component.GetAttachedShapes())
		{
			if (ShapeComponent == nullptr)
				continue;

			if (GetShapeComponentName(*ShapeComponent) == ShapeName ||
				ShapeComponent->GetFName() == ShapeName)
				return ShapeComponent;
		}

		return nullptr;
	}

	FAGX_TerrainMaterialPatchData* GetPatchDataByShapeName(
		UAGX_TerrainMaterialPatchComponent& Component, const FName& ShapeName)
	{
		if (ShapeName.IsNone())
			return nullptr;

		if (FAGX_TerrainMaterialPatchData* PatchData =
				Component.GetTerrainMaterialPatches().FindByPredicate(
					[ShapeName](const FAGX_TerrainMaterialPatchData& Data)
					{ return Data.ShapeComponentName == ShapeName; }))
		{
			return PatchData;
		}

		UAGX_ShapeComponent* Shape = GetAttachedShapeByName(Component, ShapeName);
		if (Shape == nullptr)
			return nullptr;

		const FName StoredShapeName = GetShapeComponentName(*Shape);
		return Component.GetTerrainMaterialPatches().FindByPredicate(
			[StoredShapeName](const FAGX_TerrainMaterialPatchData& Data)
			{ return Data.ShapeComponentName == StoredShapeName; });
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

	FTerrainPagerBarrier* GetTerrainPagerBarrier(UAGX_TerrainMaterialPatchComponent& Component)
	{
		AAGX_Terrain* Terrain = Cast<AAGX_Terrain>(Component.GetOwner());
		if (Terrain == nullptr)
			return nullptr;

		Terrain->GetOrCreateNative();
		return Terrain->HasNativeTerrainPager() ? Terrain->GetNativeTerrainPager() : nullptr;
	}

	struct FSetTerrainMaterialResult
	{
		bool bSuccess {false};
		TOptional<int32> NumVoxels;
	};

	FSetTerrainMaterialResult SetTerrainMaterial(
		FTerrainBarrier& TerrainBarrier, FTerrainMaterialBarrier& TerrainMaterial,
		FShapeBarrier& Shape)
	{
		const int32 NumVoxels = TerrainBarrier.SetTerrainMaterial(TerrainMaterial, Shape);
		return {NumVoxels > 0, NumVoxels};
	}

	FSetTerrainMaterialResult SetTerrainMaterial(
		FTerrainPagerBarrier& TerrainPagerBarrier, FTerrainMaterialBarrier& TerrainMaterial,
		FShapeBarrier& Shape)
	{
		return {TerrainPagerBarrier.SetTerrainMaterial(TerrainMaterial, Shape), {}};
	}

	template <typename TerrainBarrierT>
	void ApplyTerrainMaterialPatch(
		UAGX_TerrainMaterialPatchComponent& Component, const TArray<FTransform>& Transforms,
		TerrainBarrierT& TerrainBarrier, UAGX_ShapeComponent* Shape,
		UAGX_TerrainMaterial* TerrainMaterial, UAGX_ShapeMaterial* ShapeMaterial)
	{
		if (!Component.bEnabled)
			return;

		if (Shape == nullptr || TerrainMaterial == nullptr)
			return;

		FTerrainMaterialBarrier* TerrainMaterialBarrier =
			GetTerrainMaterialBarrier(TerrainMaterial, Component.GetWorld());
		if (TerrainMaterialBarrier == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Terrain Material Patch Component '%s' in '%s', unable to create Terrain "
					 "Material Barrier from Terrain Material '%s'."),
				*Component.GetName(), *GetLabelSafe(Component.GetOwner()),
				*TerrainMaterial->GetName());
			return;
		}

		const bool bShapeHadNative = Shape->HasNative();
		FShapeBarrier* ShapeBarrier = GetShapeBarrier(Shape);
		if (ShapeBarrier == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Terrain Material Patch Component '%s' in '%s', unable to create Shape "
					 "Barrier from Shape Component '%s'."),
				*Component.GetName(), *GetLabelSafe(Component.GetOwner()), *Shape->GetName());
			return;
		}

		FShapeMaterialBarrier* ShapeMaterialBarrier =
			GetShapeMaterialBarrier(ShapeMaterial, Component.GetWorld());

		const FTransform OriginalWorldTransform = Shape->GetComponentTransform();
		bool bAnyPatchAssigned = false;
		for (const FTransform& Transform : Transforms)
		{
			// Instance transforms are interpreted relative to the shape's original world transform.
			FTransform StampedWorldTransform(
				OriginalWorldTransform.TransformRotation(Transform.GetRotation()),
				OriginalWorldTransform.TransformPositionNoScale(Transform.GetLocation()),
				OriginalWorldTransform.GetScale3D() * Transform.GetScale3D());
			Shape->SetWorldTransform(StampedWorldTransform);
			Shape->UpdateNativeProperties();

			const FSetTerrainMaterialResult Result =
				SetTerrainMaterial(TerrainBarrier, *TerrainMaterialBarrier, *ShapeBarrier);
			bAnyPatchAssigned = bAnyPatchAssigned || Result.bSuccess;

			if (!Result.bSuccess)
			{
				UE_LOG(
					LogAGX, Warning,
					TEXT("ApplyTerrainMaterialPatch called on Terrain Material Patch Component "
						 "'%s' in '%s' but the patch could not be assigned when using Shape '%s' "
						 "and Terrain Material '%s'."),
					*Component.GetName(), *GetLabelSafe(Component.GetOwner()), *Shape->GetName(),
					*TerrainMaterial->GetName());
			}

			if (Component.bLogPatchAssignments && Result.bSuccess)
			{
				if (Result.NumVoxels.IsSet())
				{
					UE_LOG(
						LogAGX, Log,
						TEXT("Terrain Material Patch Component '%s' in '%s' assigned %d voxels "
							 "when using Shape '%s' and Terrain Material '%s'."),
						*Component.GetName(), *GetLabelSafe(Component.GetOwner()),
						Result.NumVoxels.GetValue(), *Shape->GetName(),
						*TerrainMaterial->GetName());
				}
				else
				{
					UE_LOG(
						LogAGX, Log,
						TEXT("Terrain Material Patch Component '%s' in '%s' registered a paged "
							 "Terrain Material patch when using Shape '%s' and Terrain Material "
							 "'%s'."),
						*Component.GetName(), *GetLabelSafe(Component.GetOwner()),
						*Shape->GetName(), *TerrainMaterial->GetName());
				}
			}
		}

		if (bAnyPatchAssigned && ShapeMaterialBarrier != nullptr)
		{
			if (!TerrainBarrier.SetAssociatedMaterial(
					*TerrainMaterialBarrier, *ShapeMaterialBarrier))
			{
				UE_LOG(
					LogAGX, Warning,
					TEXT("Terrain Material Patch Component '%s' in '%s' associating Shape "
						 "Material '%s' with Terrain Material '%s' failed. The Output Log may "
						 "contain more details."),
					*Component.GetName(), *GetLabelSafe(Component.GetOwner()),
					*ShapeMaterial->GetName(), *TerrainMaterial->GetName());
			}
		}

		if (Shape->HasNative())
		{
			if (bShapeHadNative)
			{
				// Restore the original component/native transform since we changed it during
				// "stamping" above.
				Shape->SetWorldTransform(OriginalWorldTransform);
				Shape->UpdateNativeProperties();
			}
			else
			{
				Shape->SetWorldTransform(OriginalWorldTransform);

				// Release the Shape Native since we created it. This is important since the
				// Shape may have the bIncludeInSimulation set to false, in which case it will
				// crash on Blueprint Reconstruction since no one (Simulation) is keeping it alive.
				Shape->ReleaseNative();
			}
		}
	}

	template <typename TerrainBarrierT>
	void ApplyTerrainMaterialPatch(
		UAGX_TerrainMaterialPatchComponent& Component,
		const FAGX_TerrainMaterialPatchData& PatchData, TerrainBarrierT& TerrainBarrier)
	{
		UAGX_ShapeComponent* Shape =
			GetAttachedShapeByName(Component, PatchData.ShapeComponentName);
		ApplyTerrainMaterialPatch(
			Component, PatchData.InstancePlacements, TerrainBarrier, Shape,
			PatchData.TerrainMaterial, PatchData.ShapeMaterial);
	}

	bool ApplyTerrainMaterialPatch(
		UAGX_TerrainMaterialPatchComponent& Component, const TArray<FTransform>& Transforms,
		UAGX_ShapeComponent* Shape, UAGX_TerrainMaterial* TerrainMaterial,
		UAGX_ShapeMaterial* ShapeMaterial)
	{
		if (FTerrainPagerBarrier* TerrainPagerBarrier = GetTerrainPagerBarrier(Component))
		{
			ApplyTerrainMaterialPatch(
				Component, Transforms, *TerrainPagerBarrier, Shape, TerrainMaterial, ShapeMaterial);
			return true;
		}

		FTerrainBarrier* TerrainBarrier = GetTerrainBarrier(Component);
		if (TerrainBarrier == nullptr)
			return false;

		ApplyTerrainMaterialPatch(
			Component, Transforms, *TerrainBarrier, Shape, TerrainMaterial, ShapeMaterial);
		return true;
	}

	bool ApplyTerrainMaterialPatch(
		UAGX_TerrainMaterialPatchComponent& Component,
		const FAGX_TerrainMaterialPatchData& PatchData)
	{
		if (FTerrainPagerBarrier* TerrainPagerBarrier = GetTerrainPagerBarrier(Component))
		{
			ApplyTerrainMaterialPatch(Component, PatchData, *TerrainPagerBarrier);
			return true;
		}

		if (FTerrainBarrier* TerrainBarrier = GetTerrainBarrier(Component))
		{
			ApplyTerrainMaterialPatch(Component, PatchData, *TerrainBarrier);
			return true;
		}

		return false;
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
	FName ShapeName, const FTransform& Transform)
{
	using namespace AGX_TerrainMaterialPatchComponent_helpers;
	if (ShapeName.IsNone())
		return false;

	FAGX_TerrainMaterialPatchData* PatchData = GetPatchDataByShapeName(*this, ShapeName);
	if (PatchData == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("AddPatchShapeInstance called on Terrain Material Patch Component '%s' in '%s', "
				 "but no Patch Data could be matched against given ShapeName '%s'."),
			*GetName(), *GetLabelSafe(GetOwner()), *ShapeName.ToString());
		return false;
	}

	PatchData->InstancePlacements.Add(Transform);

	if (GetWorld() == nullptr || !GetWorld()->IsGameWorld())
		return true; // Case for in editor calls.

	UAGX_ShapeComponent* Shape = GetAttachedShapeByName(*this, PatchData->ShapeComponentName);
	if (Shape == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("AddPatchShapeInstance called on Terrain Material Patch Component '%s' in '%s', "
				 "could not find attached Shape given ShapeName '%s'."),
			*GetName(), *GetLabelSafe(GetOwner()), *ShapeName.ToString());
		return false;
	}

	return AGX_TerrainMaterialPatchComponent_helpers::ApplyTerrainMaterialPatch(
		*this, {Transform}, Shape, PatchData->TerrainMaterial, PatchData->ShapeMaterial);
}

bool UAGX_TerrainMaterialPatchComponent::AddPatchShapeInstances(
	FName ShapeName, const TArray<FTransform>& Transforms)
{
	bool bAllAdded = true;
	for (const FTransform& Transform : Transforms)
	{
		const bool bAdded = AddPatchShapeInstance(ShapeName, Transform);
		bAllAdded = bAllAdded && bAdded;
	}

	return bAllAdded;
}

bool UAGX_TerrainMaterialPatchComponent::ClearShapeInstances(FName ShapeName)
{
	using namespace AGX_TerrainMaterialPatchComponent_helpers;
	if (ShapeName.IsNone())
		return false;

	FAGX_TerrainMaterialPatchData* PatchData = GetPatchDataByShapeName(*this, ShapeName);
	if (PatchData == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("ClearShapeInstances called on Terrain Material Patch Component '%s' in '%s', "
				 "but no Patch Data could be matched against given ShapeName '%s'."),
			*GetName(), *GetLabelSafe(GetOwner()), *ShapeName.ToString());
		return false;
	}

	Modify();
	PatchData->InstancePlacements.Reset();
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

	TArray<FTransform> Transforms {FTransform::Identity};
	if (!AGX_TerrainMaterialPatchComponent_helpers::ApplyTerrainMaterialPatch(
			*this, Transforms, ShapeComponent, TerrainMaterial, ShapeMaterial))
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("AddPatch called on Terrain Material Patch Component '%s' in '%s'. Unable to "
				 "find a Terrain parent, doing nothing."),
			*GetName(), *GetLabelSafe(GetOwner()));
	}
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

bool UAGX_TerrainMaterialPatchComponent::AddAssignmentDataIfMissing(
	const UAGX_ShapeComponent& ShapeComponent)
{
	const FName ShapeName =
		AGX_TerrainMaterialPatchComponent_helpers::GetShapeComponentName(ShapeComponent);
	if (ShapeName.IsNone())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("AddAssignmentDataIfMissing called AGX Terrain Material Patch Component '%s' in "
				 "'%s' with empty Shape Name."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return false;
	}

	if (TerrainMaterialPatches.FindByPredicate(
			[ShapeName](const FAGX_TerrainMaterialPatchData& Assignment)
			{ return Assignment.ShapeComponentName == ShapeName; }) != nullptr)
	{
		return true;
	}

	FAGX_TerrainMaterialPatchData& NewAssignment = TerrainMaterialPatches.AddDefaulted_GetRef();
	NewAssignment.ShapeComponentName = ShapeName;
	return true;
}

bool UAGX_TerrainMaterialPatchComponent::RemoveAssignmentDataIfPresent(
	const UAGX_ShapeComponent& ShapeComponent)
{
	const FName ShapeName =
		AGX_TerrainMaterialPatchComponent_helpers::GetShapeComponentName(ShapeComponent);
	if (ShapeName.IsNone())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("RemoveAssignmentDataIfPresent called AGX Terrain Material Patch Component '%s' "
				 "in '%s' with empty Shape Name."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return false;
	}

	TerrainMaterialPatches.RemoveAll([ShapeName](const FAGX_TerrainMaterialPatchData& Assignment)
									 { return Assignment.ShapeComponentName == ShapeName; });
	return true;
}

void UAGX_TerrainMaterialPatchComponent::BeginPlay()
{
	using namespace AGX_TerrainMaterialPatchComponent_helpers;
	Super::BeginPlay();

	if (GIsReconstructingBlueprintInstances)
		return;

	UpdateTerrainMaterialPatches();

	if (GetTerrainBarrier(*this) == nullptr)
	{
		const FString Message = FString::Printf(
			TEXT("AGX Terrain Material Patch Component '%s' in '%s' could not find an AGX Terrain "
				 "or AGX Movable Terrain parent. Ignoring terrain material patch assignments."),
			*GetName(), *GetLabelSafe(GetOwner()));
		FAGX_NotificationUtilities::ShowNotification(Message, SNotificationItem::CS_Fail);
		return;
	}

	for (const FAGX_TerrainMaterialPatchData& AssignmentData : TerrainMaterialPatches)
	{
		ApplyTerrainMaterialPatch(*this, AssignmentData);
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
