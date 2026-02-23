// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainMaterialPatchComponentVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "Shapes/AGX_BoxShapeComponent.h"
#include "Shapes/AGX_CapsuleShapeComponent.h"
#include "Shapes/AGX_CylinderShapeComponent.h"
#include "Shapes/AGX_ShapeComponent.h"
#include "Shapes/AGX_SphereShapeComponent.h"
#include "Terrain/AGX_TerrainMaterialPatchComponent.h"
#include "Utilities/AGX_BlueprintUtilities.h"

// Unreal Engine includes.
#include "SceneManagement.h"

namespace AGX_TerrainMaterialPatchComponentVisualizer_helpers
{
	FName GetShapeComponentName(const UAGX_ShapeComponent& ShapeComponent)
	{
		return FName(*FAGX_BlueprintUtilities::GetRegularNameFromTemplateComponentName(
			ShapeComponent.GetName()));
	}

	const UAGX_ShapeComponent* GetAttachedShapeByName(
		const UAGX_TerrainMaterialPatchComponent& PatchComponent, const FName& ShapeName)
	{
		if (ShapeName.IsNone())
			return nullptr;

		for (const UAGX_ShapeComponent* ShapeComponent : PatchComponent.GetAttachedShapes())
		{
			if (ShapeComponent == nullptr)
				continue;

			if (GetShapeComponentName(*ShapeComponent) == ShapeName ||
				ShapeComponent->GetFName() == ShapeName)
				return ShapeComponent;
		}

		return nullptr;
	}

	FTransform GetInstanceWorldTransform(
		const USceneComponent& Shape, const FTransform& InstanceTransform)
	{
		const FVector WorldPos =
			Shape.GetComponentTransform().TransformPositionNoScale(InstanceTransform.GetLocation());
		const FQuat WorldRot =
			Shape.GetComponentTransform().TransformRotation(InstanceTransform.GetRotation());

		return FTransform(WorldRot, WorldPos);
	}

	void DrawSphere(
		const UAGX_SphereShapeComponent& SphereShape, const FTransform& InstanceWorldTransform,
		FPrimitiveDrawInterface* PDI)
	{
		DrawWireSphere(
			PDI, InstanceWorldTransform, FLinearColor::Yellow, SphereShape.GetRadius(), 20,
			SDPG_Foreground, 1.5f);
	}

	void DrawCylinder(
		const UAGX_CylinderShapeComponent& CylinderShape, const FTransform& InstanceWorldTransform,
		FPrimitiveDrawInterface* PDI)
	{
		DrawWireCylinder(
			PDI, InstanceWorldTransform.GetLocation(), InstanceWorldTransform.GetUnitAxis(EAxis::Z),
			InstanceWorldTransform.GetUnitAxis(EAxis::X),
			InstanceWorldTransform.GetUnitAxis(EAxis::Y), FLinearColor::Yellow,
			CylinderShape.GetRadius(), 0.5f * CylinderShape.GetHeight(), 24, SDPG_Foreground);
	}

	void DrawCapsule(
		const UAGX_CapsuleShapeComponent& CapsuleShape, const FTransform& InstanceWorldTransform,
		FPrimitiveDrawInterface* PDI)
	{
		const float Radius = CapsuleShape.GetRadius();
		const float HalfHeight = 0.5f * CapsuleShape.GetHeight();

		DrawWireCylinder(
			PDI, InstanceWorldTransform.GetLocation(), InstanceWorldTransform.GetUnitAxis(EAxis::Z),
			InstanceWorldTransform.GetUnitAxis(EAxis::X),
			InstanceWorldTransform.GetUnitAxis(EAxis::Y), FLinearColor::Yellow, Radius, HalfHeight,
			24, SDPG_Foreground);

		const FVector Axis = InstanceWorldTransform.GetUnitAxis(EAxis::Z);
		const FVector Top = InstanceWorldTransform.GetLocation() + Axis * HalfHeight;
		const FVector Bottom = InstanceWorldTransform.GetLocation() - Axis * HalfHeight;
		DrawWireSphere(
			PDI, FTransform(InstanceWorldTransform.GetRotation(), Top), FLinearColor::Yellow,
			Radius, 20, SDPG_Foreground, 1.5f);
		DrawWireSphere(
			PDI, FTransform(InstanceWorldTransform.GetRotation(), Bottom), FLinearColor::Yellow,
			Radius, 20, SDPG_Foreground, 1.5f);
	}

	void DrawBoxInstances(
		const UAGX_BoxShapeComponent& BoxShape, const TArray<FTransform>& InstanceTransforms,
		FPrimitiveDrawInterface* PDI)
	{
		const FBox LocalBounds(-BoxShape.GetHalfExtent(), BoxShape.GetHalfExtent());
		for (const FTransform& InstanceTransform : InstanceTransforms)
		{
			const FTransform InstanceWorldTransform =
				GetInstanceWorldTransform(BoxShape, InstanceTransform);
			DrawWireBox(
				PDI, InstanceWorldTransform.ToMatrixNoScale(), LocalBounds, FLinearColor::Yellow,
				SDPG_Foreground, 1.5f, 0.f, true);
		}
	}

	void DrawSphereInstances(
		const UAGX_SphereShapeComponent& SphereShape, const TArray<FTransform>& InstanceTransforms,
		FPrimitiveDrawInterface* PDI)
	{
		for (const FTransform& InstanceTransform : InstanceTransforms)
		{
			const FTransform InstanceWorldTransform =
				GetInstanceWorldTransform(SphereShape, InstanceTransform);
			DrawSphere(SphereShape, InstanceWorldTransform, PDI);
		}
	}

	void DrawCylinderInstances(
		const UAGX_CylinderShapeComponent& CylinderShape,
		const TArray<FTransform>& InstanceTransforms, FPrimitiveDrawInterface* PDI)
	{
		for (const FTransform& InstanceTransform : InstanceTransforms)
		{
			const FTransform InstanceWorldTransform =
				GetInstanceWorldTransform(CylinderShape, InstanceTransform);
			DrawCylinder(CylinderShape, InstanceWorldTransform, PDI);
		}
	}

	void DrawCapsuleInstances(
		const UAGX_CapsuleShapeComponent& CapsuleShape,
		const TArray<FTransform>& InstanceTransforms, FPrimitiveDrawInterface* PDI)
	{
		for (const FTransform& InstanceTransform : InstanceTransforms)
		{
			const FTransform InstanceWorldTransform =
				GetInstanceWorldTransform(CapsuleShape, InstanceTransform);
			DrawCapsule(CapsuleShape, InstanceWorldTransform, PDI);
		}
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

	for (const FAGX_TerrainMaterialPatchData& PatchData :
		 PatchComponent->GetTerrainMaterialPatches())
	{
		if (!PatchData.bDebugRenderInstances)
			continue;

		const UAGX_ShapeComponent* Shape =
			AGX_TerrainMaterialPatchComponentVisualizer_helpers::GetAttachedShapeByName(
				*PatchComponent, PatchData.ShapeComponentName);
		if (Shape == nullptr)
			continue;

		if (const UAGX_BoxShapeComponent* BoxShape = Cast<const UAGX_BoxShapeComponent>(Shape))
		{
			AGX_TerrainMaterialPatchComponentVisualizer_helpers::DrawBoxInstances(
				*BoxShape, PatchData.InstanceTransforms, PDI);
		}
		else if (
			const UAGX_SphereShapeComponent* SphereShape =
				Cast<const UAGX_SphereShapeComponent>(Shape))
		{
			AGX_TerrainMaterialPatchComponentVisualizer_helpers::DrawSphereInstances(
				*SphereShape, PatchData.InstanceTransforms, PDI);
		}
		else if (
			const UAGX_CylinderShapeComponent* CylinderShape =
				Cast<const UAGX_CylinderShapeComponent>(Shape))
		{
			AGX_TerrainMaterialPatchComponentVisualizer_helpers::DrawCylinderInstances(
				*CylinderShape, PatchData.InstanceTransforms, PDI);
		}
		else if (
			const UAGX_CapsuleShapeComponent* CapsuleShape =
				Cast<const UAGX_CapsuleShapeComponent>(Shape))
		{
			AGX_TerrainMaterialPatchComponentVisualizer_helpers::DrawCapsuleInstances(
				*CapsuleShape, PatchData.InstanceTransforms, PDI);
		}
	}
}
