// Copyright 2026, Algoryx Simulation AB.

#include "Wire/AGX_WireLinkComponentVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyComponent.h"
#include "Utilities/AGX_SlateUtilities.h"
#include "Wire/AGX_WireLinkComponent.h"

// Unreal Engine includes.
#include "SceneManagement.h"
#include "SceneView.h"

namespace
{
	float GetWireLinkCircleRadius(const FSceneView& View, const FVector& Location)
	{
		static constexpr float ScreenScale = 0.025f;
		static constexpr float MaxDistance = 10000.0f;
		static constexpr float MinRadius = 5.0f;

		const float Distance = FMath::Min(FVector::Dist(Location, View.ViewLocation), MaxDistance);
		const float Radius =
			ScreenScale * Distance * FMath::Tan(FMath::DegreesToRadians(View.FOV) / 2.0f);
		return FMath::Max(Radius, MinRadius);
	}
}

void FAGX_WireLinkComponentVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UAGX_WireLinkComponent* WireLinkComponent = Cast<const UAGX_WireLinkComponent>(Component);
	if (WireLinkComponent == nullptr || View == nullptr || PDI == nullptr)
		return;

	const UAGX_RigidBodyComponent* Body = WireLinkComponent->GetRigidBody();
	if (Body == nullptr)
		return;

	static const FColor Color = FAGX_SlateUtilities::GetAGXColorOrange();
	static constexpr float Thickness = 1.5f;

	const FVector Location = Body->GetComponentLocation();
	const float Radius = GetWireLinkCircleRadius(*View, Location);
	DrawCircle(
		PDI, Location, View->GetViewRight(), View->GetViewUp(), Color, Radius, 32,
		SDPG_Foreground, Thickness, /*DepthBias*/ 0.0f, /*bScreenSpace*/ true);
}
