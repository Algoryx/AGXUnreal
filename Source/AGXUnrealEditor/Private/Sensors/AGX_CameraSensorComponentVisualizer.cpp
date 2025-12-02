// Copyright 2025, Algoryx Simulation AB.

#include "Sensors/AGX_CameraSensorComponentVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_CameraSensorBase.h"
#include "Utilities/AGX_SlateUtilities.h"

// Unreal Engine includes.
#include "SceneView.h"
#include "SceneManagement.h"

#define LOCTEXT_NAMESPACE "FAGX_CameraSensorComponentVisualizer"

void FAGX_CameraSensorComponentVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UAGX_CameraSensorBase* Cam = Cast<const UAGX_CameraSensorBase>(Component);
	if (Cam == nullptr || !Cam->ShouldRender())
		return;

	if (!UAGX_CameraSensorBase::IsFovValid(Cam->FOV))
		return;

	if (!UAGX_CameraSensorBase::IsResolutionValid(Cam->Resolution))
		return;

	// We will draw a rectangle on an imaginary plane accordint to the FOV of the Camera Sensor.
	// Also, we will draw lines from the Camera Sensor origin to each corner of the ractangle.
	static constexpr double PlaneDistance = 40.0;

	const double FOVRad = FMath::DegreesToRadians(static_cast<double>(Cam->FOV));
	const double HalfWidth = FMath::Tan(FOVRad / 2.0) * PlaneDistance;
	const double AspectRatioInv =
		static_cast<double>(Cam->Resolution.Y) / static_cast<double>(Cam->Resolution.X);
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
