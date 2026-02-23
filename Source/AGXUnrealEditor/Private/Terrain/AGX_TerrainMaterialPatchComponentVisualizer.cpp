// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainMaterialPatchComponentVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Terrain/AGX_TerrainMaterialPatchComponent.h"
#include "Utilities/AGX_BlueprintUtilities.h"

// Unreal Engine includes.
#include "SceneManagement.h"

namespace AGX_TerrainMaterialPatchComponentVisualizer_helpers
{
	FName GetShapeComponentName(const UAGX_BoxShapeComponent& ShapeComponent)
	{
		return FName(*FAGX_BlueprintUtilities::GetRegularNameFromTemplateComponentName(
			ShapeComponent.GetName()));
	}

	const UAGX_BoxShapeComponent* GetAttachedBoxShapeByName(
		const UAGX_TerrainMaterialPatchComponent& PatchComponent, const FName& ShapeName)
	{
		if (ShapeName.IsNone())
		{
			return nullptr;
		}

		for (const UAGX_ShapeComponent* ShapeComponent : PatchComponent.GetAttachedShapes())
		{
			const UAGX_BoxShapeComponent* BoxShape =
				Cast<const UAGX_BoxShapeComponent>(ShapeComponent);
			if (BoxShape == nullptr)
			{
				continue;
			}

			if (GetShapeComponentName(*BoxShape) == ShapeName || BoxShape->GetFName() == ShapeName)
			{
				return BoxShape;
			}
		}

		return nullptr;
	}

	FTransform GetInstanceWorldTransform(
		const UAGX_BoxShapeComponent& BoxShape, const FTransform& InstanceTransform)
	{
		const FTransform InstanceRelativeTransform =
			BoxShape.GetRelativeTransform() * InstanceTransform;
		if (const USceneComponent* Parent = BoxShape.GetAttachParent())
		{
			return InstanceRelativeTransform * Parent->GetComponentTransform();
		}

		return InstanceRelativeTransform;
	}
}

void FAGX_TerrainMaterialPatchComponentVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UAGX_TerrainMaterialPatchComponent* PatchComponent =
		Cast<const UAGX_TerrainMaterialPatchComponent>(Component);
	if (PatchComponent == nullptr)
	{
		return;
	}

	for (const FAGX_TerrainMaterialPatchData& PatchData : PatchComponent->GetTerrainMaterialPatches())
	{
		if (!PatchData.bDebugRenderInstances)
		{
			continue;
		}

		const UAGX_BoxShapeComponent* BoxShape =
			AGX_TerrainMaterialPatchComponentVisualizer_helpers::GetAttachedBoxShapeByName(
				*PatchComponent, PatchData.ShapeComponentName);
		if (BoxShape == nullptr)
		{
			continue;
		}

		const FBox LocalBounds(-BoxShape->GetHalfExtent(), BoxShape->GetHalfExtent());
		for (const FTransform& InstanceTransform : PatchData.InstanceTransforms)
		{
			const FTransform InstanceWorldTransform =
				AGX_TerrainMaterialPatchComponentVisualizer_helpers::GetInstanceWorldTransform(
					*BoxShape, InstanceTransform);
			DrawWireBox(
				PDI, InstanceWorldTransform.ToMatrixNoScale(), LocalBounds, FLinearColor::Yellow,
				SDPG_Foreground, 1.5f, 0.f, true);
		}
	}
}
