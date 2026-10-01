// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_CameraSensorComponentVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_CameraSensorComponent.h"
#include "Utilities/AGX_SlateUtilities.h"

// Unreal Engine includes.
#include "SceneManagement.h"
#include "SceneView.h"

#define LOCTEXT_NAMESPACE "FAGX_CameraSensorComponentVisualizer"

void FAGX_CameraSensorComponentVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UAGX_CameraSensorComponent* Cam = Cast<const UAGX_CameraSensorComponent>(Component);
	if (Cam == nullptr || !Cam->ShouldRender())
		return;

	// We will draw a rectangle on an imaginary plane according to the FOV of the Camera Sensor.
	// Also, we will draw lines from the Camera Sensor origin to each corner of the rectangle.
	static constexpr double PlaneDistance = 30.0;
	static constexpr double FOV = 70.0;

	const double FOVRad = FMath::DegreesToRadians(FOV);
	const double HalfWidth = FMath::Tan(FOVRad / 2.0) * PlaneDistance;
	const double AspectRatioInv = 9.0/16.0;
	const double HalfHeight = HalfWidth * AspectRatioInv;
	const FTransform& Transform = Cam->GetComponentTransform();

	// Camera Sensor forward direction is x, up is z.
	const FVector Origin = Transform.GetLocation();
	const FVector Corner0 =
		Transform.TransformPositionNoScale(FVector(PlaneDistance, -HalfWidth, HalfHeight));
	const FVector Corner1 =
		Transform.TransformPositionNoScale(FVector(PlaneDistance, HalfWidth, HalfHeight));
	const FVector Corner2 =
		Transform.TransformPositionNoScale(FVector(PlaneDistance, HalfWidth, -HalfHeight));
	const FVector Corner3 =
		Transform.TransformPositionNoScale(FVector(PlaneDistance, -HalfWidth, -HalfHeight));

	const static FColor Color = FAGX_SlateUtilities::GetAGXColorOrange();

	// Camera housing.
	static const FVector HousingHalfExtent {16.0, 10.0, 10.0};
	DrawOrientedWireBox(
		PDI, Origin, Cam->GetForwardVector(), Cam->GetRightVector(), Cam->GetUpVector(),
		HousingHalfExtent, Color, SDPG_World);

	// Rectangle.
	PDI->DrawLine(Corner0, Corner1, Color, SDPG_World);
	PDI->DrawLine(Corner1, Corner2, Color, SDPG_World);
	PDI->DrawLine(Corner2, Corner3, Color, SDPG_World);
	PDI->DrawLine(Corner3, Corner0, Color, SDPG_World);

	// Origin to corners.
	PDI->DrawLine(Origin, Corner0, Color, SDPG_World);
	PDI->DrawLine(Origin, Corner1, Color, SDPG_World);
	PDI->DrawLine(Origin, Corner2, Color, SDPG_World);
	PDI->DrawLine(Origin, Corner3, Color, SDPG_World);
}

#undef LOCTEXT_NAMESPACE
