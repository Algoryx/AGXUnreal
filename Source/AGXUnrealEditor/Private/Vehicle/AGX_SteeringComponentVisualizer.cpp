// Copyright 2025, Algoryx Simulation AB.

#include "Vehicle/AGX_SteeringComponentVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyComponent.h"
#include "Vehicle/AGX_SteeringComponent.h"
#include "Vehicle/AGX_AckermannSteeringParameters.h"
#include "Vehicle/AGX_BellCrankSteeringParameters.h"
#include "Vehicle/AGX_DavisSteeringParameters.h"
#include "Vehicle/AGX_RackPinionSteeringParameters.h"
#include "Vehicle/AGX_WheelJointComponent.h"

// Unreal Engine includes.
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "SceneManagement.h"

namespace AGX_SteeringComponentVisualizer_helpers
{
	void DrawAckermannSteering(
		const UAGX_SteeringComponent& Comp, const UAGX_SteeringParameters& Params,
		const FSceneView* View, FPrimitiveDrawInterface* PDI)
	{
		UAGX_WheelJointComponent* LeftWheel = Comp.LeftWheelJoint.GetWheelJointComponent();
		UAGX_WheelJointComponent* RightWheel = Comp.RightWheelJoint.GetWheelJointComponent();
		if (LeftWheel == nullptr || RightWheel == nullptr)
			return;

		UAGX_RigidBodyComponent* LeftBody = LeftWheel->BodyAttachment1.GetRigidBody();
		UAGX_RigidBodyComponent* RightBody = RightWheel->BodyAttachment1.GetRigidBody();
		UAGX_RigidBodyComponent* ChassisBody = LeftWheel->BodyAttachment2.GetRigidBody();
		if (LeftBody == RightBody || LeftBody == nullptr || RightBody == nullptr ||
			ChassisBody == nullptr)
			return;

		const FLinearColor Color_Pin = FLinearColor(0.0f / 255.f, 117.f / 255.f, 220.f / 255.f);
		const FLinearColor Color_Rod = FLinearColor(76.f / 255.f, 0.0f / 255.f, 92.f / 255.f);
		const FLinearColor Color_WheelTrack = FLinearColor(255.f / 255.f, 0.f / 255.f, 0.f / 255.f);

		// Draw wheel base line.
		PDI->DrawLine(
			RightBody->GetPosition(), LeftBody->GetPosition(), Color_WheelTrack, SDPG_Foreground,
			1.5f);

		const double WheelBaseLen = (LeftBody->GetPosition() - RightBody->GetPosition()).Length();
		const FTransform WheelAttachmentLeft(LeftWheel->BodyAttachment1.GetGlobalFrameMatrix());
		const FTransform WheelAttachmentRight(RightWheel->BodyAttachment1.GetGlobalFrameMatrix());
		const FVector KingpinDirLocalLeft = FVector::RightVector.RotateAngleAxis(Params.SteeringData.Phi0, FVector::UpVector);
		const FVector KingpinDirLocalRight =
			(-FVector::RightVector).RotateAngleAxis(-Params.SteeringData.Phi0, FVector::UpVector);
		const FVector KingpinLeftDir = WheelAttachmentLeft.TransformVectorNoScale(KingpinDirLocalLeft);
		const FVector KingpinRightDir = WheelAttachmentRight.TransformVectorNoScale(KingpinDirLocalRight);
		
		const double KingpinLength = Params.SteeringData.L * WheelBaseLen;
		const FVector KnuckeLeftPos =
			WheelAttachmentLeft.GetLocation() + KingpinLeftDir * KingpinLength;
		const FVector KnuckeRightPos =
			WheelAttachmentRight.GetLocation() + KingpinRightDir * KingpinLength;

		// Draw kingpins.
		PDI->DrawLine(
			WheelAttachmentLeft.GetLocation(), KnuckeLeftPos, Color_Pin, SDPG_Foreground,
			1.5f);
		PDI->DrawLine(
			WheelAttachmentRight.GetLocation(), KnuckeRightPos, Color_Pin, SDPG_Foreground,
			1.5f);

		// Draw rack rod.
		PDI->DrawLine(KnuckeLeftPos, KnuckeRightPos, Color_Rod, SDPG_Foreground, 1.5f);
	}


	void DrawBellCrankSteering(
		const UAGX_SteeringComponent& Comp, const UAGX_SteeringParameters& Params,
		const FSceneView* View, FPrimitiveDrawInterface* PDI)
	{
		
	}

	void DrawDavisSteering(
		const UAGX_SteeringComponent& Comp, const UAGX_SteeringParameters& Params,
		const FSceneView* View, FPrimitiveDrawInterface* PDI)
	{
	}

	void DrawRackPinionSteering(
		const UAGX_SteeringComponent& Comp, const UAGX_SteeringParameters& Params,
		const FSceneView* View, FPrimitiveDrawInterface* PDI)
	{
	}
}

void FAGX_SteeringComponentVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	using namespace AGX_SteeringComponentVisualizer_helpers;
	const UAGX_SteeringComponent* SteeringComponent = Cast<const UAGX_SteeringComponent>(Component);
	if (SteeringComponent == nullptr || !SteeringComponent->Visible ||
		SteeringComponent->SteeringParameters == nullptr)
	{
		return;
	}

	if (SteeringComponent->GetWorld() != nullptr && SteeringComponent->GetWorld()->IsGameWorld())
		return; // Only do pre-play visualization.

	if (SteeringComponent->SteeringParameters->IsA<UAGX_AckermannSteeringParameters>())
	{
		DrawAckermannSteering(
			*SteeringComponent, *SteeringComponent->SteeringParameters, View, PDI);
	}
	else if (SteeringComponent->SteeringParameters->IsA<UAGX_BellCrankSteeringParameters>())
	{
		DrawBellCrankSteering(
			*SteeringComponent, *SteeringComponent->SteeringParameters, View, PDI);
	}
	else if (SteeringComponent->SteeringParameters->IsA<UAGX_DavisSteeringParameters>())
	{
		DrawDavisSteering(*SteeringComponent, *SteeringComponent->SteeringParameters, View, PDI);
	}
	else if (SteeringComponent->SteeringParameters->IsA<UAGX_RackPinionSteeringParameters>())
	{
		DrawRackPinionSteering(
			*SteeringComponent, *SteeringComponent->SteeringParameters, View, PDI);
	}
}
