// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_IMUSensorComponentVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_IMUSensorComponent.h"
#include "Utilities/AGX_SlateUtilities.h"

// Unreal Engine includes.
#include "SceneView.h"
#include "SceneManagement.h"

#define LOCTEXT_NAMESPACE "FAGX_IMUSensorComponentVisualizer"

void FAGX_IMUSensorComponentVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UAGX_IMUSensorComponent* IMU = Cast<const UAGX_IMUSensorComponent>(Component);
	if (IMU == nullptr || !IMU->ShouldRender())
		return;

	const static FColor Color = FAGX_SlateUtilities::GetAGXColorOrange();
	const FVector OriginPos = IMU->GetComponentLocation();
	static const FVector HalfExtent {10.0, 10.0, 5.0};

	const FVector Forward = IMU->GetForwardVector();
	const FVector Right = IMU->GetRightVector();
	const FVector Up = IMU->GetUpVector();

	// Box.
	DrawOrientedWireBox(PDI, OriginPos, Forward, Right, Up, HalfExtent, Color, SDPG_Foreground);

	// Arrows.
	auto DrawArrow = [&](const FVector& Direction, float Offset)
	{
		static constexpr float ArrowLength = 7.0f;
		static constexpr float ConeLength = 3.0f;
		static constexpr float ConeAngle = 20.0f;
		static constexpr int32 ConeSides = 16;

		const FVector Start = OriginPos + Direction * Offset;
		const FVector End = Start + Direction * ArrowLength;

		// Shaft.
		PDI->DrawLine(Start, End, Color, SDPG_Foreground);

		FTransform ConeTransform;
		ConeTransform.SetLocation(End);
		ConeTransform.SetRotation(FQuat::FindBetweenNormals(FVector::ForwardVector, -Direction));
		TArray<FVector> Unused;

		// Arrow head.
		DrawWireCone(
			PDI, Unused, ConeTransform, ConeLength, ConeAngle, ConeSides, Color, SDPG_Foreground);
	};

	DrawArrow(IMU->GetForwardVector(), HalfExtent.X);
	DrawArrow(IMU->GetRightVector(), HalfExtent.Y);
	DrawArrow(IMU->GetUpVector(), HalfExtent.Z);
}

#undef LOCTEXT_NAMESPACE
