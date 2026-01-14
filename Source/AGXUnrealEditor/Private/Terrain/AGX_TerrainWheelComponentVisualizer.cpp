// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainWheelComponentVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "Terrain/AGX_TerrainWheelComponent.h"
#include "AGX_RigidBodyComponent.h"

// Unreal Engine includes.
#include "SceneView.h"
#include "SceneManagement.h"

#define LOCTEXT_NAMESPACE "FAGX_TerrainWheelComponentVisualizer"

namespace
{
	void DrawTerrainWheelPrimitive(
		const FTransform& TerrainWheelTransform, float Radius, float Wdith, const FSceneView* View,
		FPrimitiveDrawInterface* PDI, FColor Color)
	{
		if (Radius <= 0.f)
			return;

		constexpr int32 NUM_SIDES {64};
		const float OuterCylinderHalfHeight = 0.5f * Wdith;

		DrawWireCylinder(
			PDI, TerrainWheelTransform.GetLocation(), TerrainWheelTransform.GetUnitAxis(EAxis::Z),
			TerrainWheelTransform.GetUnitAxis(EAxis::X),
			TerrainWheelTransform.GetUnitAxis(EAxis::Y), Color, Radius, OuterCylinderHalfHeight,
			NUM_SIDES, SDPG_Foreground);

		const float HalfLineLength = Radius * 0.4f;
		const float LineThickness = 1.f;
		const FVector LineStart = TerrainWheelTransform.GetLocation() +
								  TerrainWheelTransform.GetUnitAxis(EAxis::Y) * HalfLineLength;
		const FVector LineEnd = TerrainWheelTransform.GetLocation() -
								TerrainWheelTransform.GetUnitAxis(EAxis::Y) * HalfLineLength;

		PDI->DrawLine(LineStart, LineEnd, Color, SDPG_Foreground, LineThickness);
	}

	void DrawTerrainWheel(
		const UAGX_TerrainWheelComponent* TerrainWheel, const FSceneView* View,
		FPrimitiveDrawInterface* PDI)
	{
		if (TerrainWheel == nullptr)
			return;

		// We assume the body transform to allign with the Terrain Wheel transform in AGX.
		const UAGX_RigidBodyComponent* Body = TerrainWheel->RigidBody.GetRigidBody();
		if (Body == nullptr)
			return;

		const FColor TerrainWheelPrimitiveColor(140, 230, 50);
		const FTransform Transform(Body->GetRotation(), Body->GetPosition());
		DrawTerrainWheelPrimitive(
			Transform, static_cast<float>(TerrainWheel->Radius),
			static_cast<float>(TerrainWheel->Width), View, PDI, TerrainWheelPrimitiveColor);
	}
}

void FAGX_TerrainWheelComponentVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UAGX_TerrainWheelComponent* TerrainWheel =
		Cast<const UAGX_TerrainWheelComponent>(Component);
	if (TerrainWheel == nullptr || TerrainWheel->Visible == false)
		return;

	DrawTerrainWheel(TerrainWheel, View, PDI);
}

#undef LOCTEXT_NAMESPACE
