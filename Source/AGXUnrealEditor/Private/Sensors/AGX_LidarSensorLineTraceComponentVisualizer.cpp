// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_LidarSensorLineTraceComponentVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarSensorLineTraceComponent.h"

// Unreal Engine includes.
#include "SceneView.h"
#include "SceneManagement.h"

#define LOCTEXT_NAMESPACE "FAGX_LidarSensorLineTraceComponentVisualizer"

void FAGX_LidarSensorLineTraceComponentVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UAGX_LidarSensorLineTraceComponent* Lidar = Cast<const UAGX_LidarSensorLineTraceComponent>(Component);
	if (Lidar == nullptr || !Lidar->ShouldRender())
		return;

	static constexpr FColor Color {243, 139, 0};
	static constexpr float Radius {10.f};

	DrawWireCylinder(
		PDI, Lidar->GetComponentLocation(), Lidar->GetForwardVector(), Lidar->GetRightVector(),
		Lidar->GetUpVector(), Color, Radius, Radius / 2.f, 32, SDPG_Foreground);
}

#undef LOCTEXT_NAMESPACE
