// Copyright 2025, Algoryx Simulation AB.

#include "Sensors/AGX_LidarSensorComponentVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarSensorComponent.h"
#include "Utilities/AGX_SlateUtilities.h"

// Unreal Engine includes.
#include "SceneView.h"
#include "SceneManagement.h"

#define LOCTEXT_NAMESPACE "FAGX_LidarSensorComponentVisualizer"

void FAGX_LidarSensorComponentVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UAGX_LidarSensorComponent* Lidar = Cast<const UAGX_LidarSensorComponent>(Component);
	if (Lidar == nullptr || !Lidar->ShouldRender())
		return;

	const static FColor Color = FAGX_SlateUtilities::GetAGXColorOrange();
	static constexpr float Radius {10.f};

	const FVector OriginPos = Lidar->GetComponentLocation();

	DrawWireCylinder(
		PDI, OriginPos, Lidar->GetForwardVector(), Lidar->GetRightVector(), Lidar->GetUpVector(),
		Color, Radius, Radius / 2.f, 32, SDPG_Foreground);

	const FVector DirectionEndPos = Lidar->GetComponentTransform().TransformPositionNoScale(
		FVector(3.0 * static_cast<double>(Radius), 0.0, 0.0));

	PDI->DrawLine(OriginPos, DirectionEndPos, Color, SDPG_Foreground);

	TArray<FVector> Unused;
	static constexpr double ConeHeight = 5.0;
	static constexpr double ConeAngle = 30.0;
	static constexpr int32 NumSides = 32;
	FTransform ArrowTransform(Lidar->GetComponentTransform());
	ArrowTransform.SetScale3D(FVector(1.0, 1.0, 1.0));
	ArrowTransform.SetLocation(DirectionEndPos);
	DrawWireCone(
		PDI, Unused, ArrowTransform, -ConeHeight, ConeAngle, NumSides, Color,
		SDPG_Foreground);
}

#undef LOCTEXT_NAMESPACE
