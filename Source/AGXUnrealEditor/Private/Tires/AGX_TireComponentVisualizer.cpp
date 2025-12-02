// Copyright 2025, Algoryx Simulation AB.

#include "Tires/AGX_TireComponentVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "Tires/AGX_TwoBodyTireComponent.h"
#include "AGX_RigidBodyComponent.h"

// Unreal Engine includes.
#include "SceneView.h"
#include "SceneManagement.h"

#define LOCTEXT_NAMESPACE "FAGX_TireComponentVisualizer"

namespace
{
	void DrawTirePrimitive(
		const FTransform& TireTransform, float OuterRadius, float InnerRadius,
		const FSceneView* View, FPrimitiveDrawInterface* PDI, FColor Color)
	{
		if (OuterRadius <= 0.f || InnerRadius <= 0.f)
		{
			return;
		}

		constexpr int32 NUM_SIDES {64};
		const float OuterCylinderHalfHeight = 0.1f * OuterRadius;
		const float InnerCylinderHalfHeight = 0.1f * InnerRadius;

		// Note: The agx::Tire model is implemented in such a way that it assumes that the axis of
		// rotation of the tire is along the y-axis of the final TireTransform that is given by the
		// transform of the tire RigidBody plus a local offset position and rotation. Therefore we
		// draw the Tire primitives along the y-axis of the TireTransform. We have to consider the
		// different coordinate systems used in AGX Dynamics vs Unreal; AGX Dynamics y-axis goes in
		// Unreal's negative y-axis direction, but since we are drawing a symmetric collection of
		// primitives with no offset from the origin there is no difference in this case.
		DrawWireCylinder(
			PDI, TireTransform.GetLocation(), TireTransform.GetUnitAxis(EAxis::Z),
			TireTransform.GetUnitAxis(EAxis::X), TireTransform.GetUnitAxis(EAxis::Y), Color,
			OuterRadius, OuterCylinderHalfHeight, NUM_SIDES, SDPG_Foreground);

		DrawWireCylinder(
			PDI, TireTransform.GetLocation(), TireTransform.GetUnitAxis(EAxis::Z),
			TireTransform.GetUnitAxis(EAxis::X), TireTransform.GetUnitAxis(EAxis::Y), Color,
			InnerRadius, InnerCylinderHalfHeight, NUM_SIDES, SDPG_Foreground);

		const float HalfLineLength = OuterRadius * 0.4;
		const float LineThickness = 1.f;
		const FVector LineStart =
			TireTransform.GetLocation() + TireTransform.GetUnitAxis(EAxis::Y) * HalfLineLength;
		const FVector LineEnd =
			TireTransform.GetLocation() - TireTransform.GetUnitAxis(EAxis::Y) * HalfLineLength;

		PDI->DrawLine(LineStart, LineEnd, Color, SDPG_Foreground, LineThickness);
	}

	void DrawTwoBodyTire(
		const UAGX_TwoBodyTireComponent* Tire, const FSceneView* View, FPrimitiveDrawInterface* PDI)
	{
		if (Tire == nullptr)
		{
			return;
		}

		if (Tire->GetTireRigidBody() && Tire->GetHubRigidBody())
		{
			const FColor TirePrimitiveColor(240, 230, 0);

			DrawTirePrimitive(
				Tire->GetGlobalTireTransform(), Tire->OuterRadius, Tire->InnerRadius, View, PDI,
				TirePrimitiveColor);
		}
	}
}

void FAGX_TireComponentVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UAGX_TwoBodyTireComponent* Tire = Cast<const UAGX_TwoBodyTireComponent>(Component);
	if (Tire == nullptr || Tire->Visible == false)
	{
		return;
	}

	DrawTwoBodyTire(Tire, View, PDI);
}

#undef LOCTEXT_NAMESPACE
