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
	FLinearColor GetKingpinColor()
	{
		return FLinearColor(0.0f, 117.f / 255.f, 220.f / 255.f);
	}

	FLinearColor GetRackColor()
	{
		return FLinearColor(76.f / 255.f, 0.0f, 92.f / 255.f);
	}

	FLinearColor GetWheelbaseColor()
	{
		return FLinearColor(1.f, 0.f, 0.f);
	}

	FLinearColor GetSteeringArmColor()
	{
		return GetWheelbaseColor();
	}

	FLinearColor GetSphereColor()
	{
		return FLinearColor(1.f, 0.9f, 0.f);
	}

	FLinearColor GetCentralRackColor()
	{
		return GetSphereColor();
	}

	void GetKnucklePositions(
		const FTransform& AttachmentLeft, const FTransform& AttachmentRight,
		const UAGX_SteeringParameters& Params, FVector& OutLeft, FVector& OutRight)
	{
		const double WheelBaseLen =
			(AttachmentLeft.GetLocation() - AttachmentRight.GetLocation()).Length();
		const FVector KingpinDirLocalLeft =
			FVector::RightVector.RotateAngleAxis(Params.SteeringData.Phi0, FVector::UpVector);
		const FVector KingpinDirLocalRight =
			(-FVector::RightVector).RotateAngleAxis(-Params.SteeringData.Phi0, FVector::UpVector);
		const FVector KingpinLeftDir = AttachmentLeft.TransformVectorNoScale(KingpinDirLocalLeft);
		const FVector KingpinRightDir =
			AttachmentRight.TransformVectorNoScale(KingpinDirLocalRight);

		const double KingpinLength = Params.SteeringData.L * WheelBaseLen;
		OutLeft = AttachmentLeft.GetLocation() + KingpinLeftDir * KingpinLength;
		OutRight = AttachmentRight.GetLocation() + KingpinRightDir * KingpinLength;
	}

	bool VerifyValidAttachments(
		UAGX_WheelJointComponent* LeftWheel, UAGX_WheelJointComponent* RightWheel)
	{
		if (LeftWheel == nullptr || RightWheel == nullptr)
			return false;

		UAGX_RigidBodyComponent* LeftBody = LeftWheel->BodyAttachment1.GetRigidBody();
		UAGX_RigidBodyComponent* RightBody = RightWheel->BodyAttachment1.GetRigidBody();
		UAGX_RigidBodyComponent* ChassisBody = LeftWheel->BodyAttachment2.GetRigidBody();

		return LeftBody != RightBody && LeftBody != nullptr && RightBody != nullptr &&
			   ChassisBody != nullptr;
	}

	void DrawAckermannSteering(
		const UAGX_SteeringComponent& Comp, const UAGX_SteeringParameters& Params,
		const FSceneView* View, FPrimitiveDrawInterface* PDI)
	{
		UAGX_WheelJointComponent* LeftWheel = Comp.LeftWheelJoint.GetWheelJointComponent();
		UAGX_WheelJointComponent* RightWheel = Comp.RightWheelJoint.GetWheelJointComponent();
		if (!VerifyValidAttachments(LeftWheel, RightWheel))
			return;

		const FTransform AttachmentLeft(LeftWheel->BodyAttachment1.GetGlobalFrameMatrix());
		const FTransform AttachmentRight(RightWheel->BodyAttachment1.GetGlobalFrameMatrix());
		FVector KnuckleLeftPos, KnuckleRightPos;
		GetKnucklePositions(
			AttachmentLeft, AttachmentRight, Params, KnuckleLeftPos, KnuckleRightPos);

		// Draw wheel base line.
		PDI->DrawLine(
			AttachmentLeft.GetLocation(), AttachmentRight.GetLocation(), GetWheelbaseColor(),
			SDPG_Foreground, 2.f);

		// Draw kingpins.
		PDI->DrawLine(
			AttachmentLeft.GetLocation(), KnuckleLeftPos, GetKingpinColor(), SDPG_Foreground, 2.f);
		PDI->DrawLine(
			AttachmentRight.GetLocation(), KnuckleRightPos, GetKingpinColor(), SDPG_Foreground,
			2.f);

		// Draw rack rod.
		PDI->DrawLine(KnuckleLeftPos, KnuckleRightPos, GetRackColor(), SDPG_Foreground, 2.f);
	}

	void DrawBellCrankSteering(
		const UAGX_SteeringComponent& Comp, const UAGX_SteeringParameters& Params,
		const FSceneView* View, FPrimitiveDrawInterface* PDI)
	{
		UAGX_WheelJointComponent* LeftWheel = Comp.LeftWheelJoint.GetWheelJointComponent();
		UAGX_WheelJointComponent* RightWheel = Comp.RightWheelJoint.GetWheelJointComponent();
		if (!VerifyValidAttachments(LeftWheel, RightWheel))
			return;

		const FTransform AttachmentLeft(LeftWheel->BodyAttachment1.GetGlobalFrameMatrix());
		const FTransform AttachmentRight(RightWheel->BodyAttachment1.GetGlobalFrameMatrix());
		FVector KnuckleLeftPos, KnuckleRightPos;
		GetKnucklePositions(
			AttachmentLeft, AttachmentRight, Params, KnuckleLeftPos, KnuckleRightPos);

		FVector KnuckleToSteeringArmStartDir = (KnuckleRightPos - KnuckleLeftPos).GetSafeNormal();
		KnuckleToSteeringArmStartDir = FQuat(
										   AttachmentLeft.GetRotation().GetUpVector(),
										   FMath::DegreesToRadians(-Params.SteeringData.Alpha0))
										   .RotateVector(KnuckleToSteeringArmStartDir);
		const double KnuckleToKnucleDist = (KnuckleLeftPos - KnuckleRightPos).Length();

		const FVector SteeringArmStart =
			KnuckleLeftPos + KnuckleToSteeringArmStartDir * 0.5 * KnuckleToKnucleDist /
								 FMath::Cos(FMath::DegreesToRadians(Params.SteeringData.Alpha0));
		const double KingpinLen = (AttachmentLeft.GetLocation() - KnuckleLeftPos).Length();
		const FVector SteeringArmDir = -AttachmentLeft.GetRotation().GetAxisX();
		const FVector SteeringArmEnd =
			SteeringArmStart + SteeringArmDir * KingpinLen * Params.SteeringData.Lc;

		// Draw kingpins.
		PDI->DrawLine(
			AttachmentLeft.GetLocation(), KnuckleLeftPos, GetKingpinColor(), SDPG_Foreground, 2.f);
		PDI->DrawLine(
			AttachmentRight.GetLocation(), KnuckleRightPos, GetKingpinColor(), SDPG_Foreground,
			2.f);

		// Draw rack rods.
		PDI->DrawLine(KnuckleLeftPos, SteeringArmStart, GetRackColor(), SDPG_Foreground, 2.f);
		PDI->DrawLine(KnuckleRightPos, SteeringArmStart, GetRackColor(), SDPG_Foreground, 2.f);

		// Draw steering arm.
		PDI->DrawLine(
			SteeringArmStart, SteeringArmEnd, GetSteeringArmColor(), SDPG_Foreground, 2.f);
		DrawWireSphere(
			PDI, FTransform(FQuat::Identity, SteeringArmStart), GetSphereColor(), 3.f, 8,
			SDPG_Foreground, 1.f);
		DrawWireSphere(
			PDI, FTransform(FQuat::Identity, SteeringArmEnd), GetSphereColor(), 3.f, 8,
			SDPG_Foreground, 1.f);
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
		UAGX_WheelJointComponent* LeftWheel = Comp.LeftWheelJoint.GetWheelJointComponent();
		UAGX_WheelJointComponent* RightWheel = Comp.RightWheelJoint.GetWheelJointComponent();
		if (!VerifyValidAttachments(LeftWheel, RightWheel))
			return;

		const FTransform AttachmentLeft(LeftWheel->BodyAttachment1.GetGlobalFrameMatrix());
		const FTransform AttachmentRight(RightWheel->BodyAttachment1.GetGlobalFrameMatrix());
		FVector KnuckleLeftPos, KnuckleRightPos;
		GetKnucklePositions(
			AttachmentLeft, AttachmentRight, Params, KnuckleLeftPos, KnuckleRightPos);

		FVector KnuckleToSteeringArmStartDir = (KnuckleRightPos - KnuckleLeftPos).GetSafeNormal();
		KnuckleToSteeringArmStartDir = FQuat(
										   AttachmentLeft.GetRotation().GetUpVector(),
										   FMath::DegreesToRadians(-Params.SteeringData.Alpha0))
										   .RotateVector(KnuckleToSteeringArmStartDir);
		const double KnuckleToKnucleDist = (KnuckleLeftPos - KnuckleRightPos).Length();
		const double WheelBaseLen =
			(AttachmentLeft.GetLocation() - AttachmentRight.GetLocation()).Length();
		const double RackLen = WheelBaseLen * Params.SteeringData.Lr;

		const FVector RackStart =
			KnuckleLeftPos + KnuckleToSteeringArmStartDir * 0.5 * (KnuckleToKnucleDist - RackLen) /
								 FMath::Cos(FMath::DegreesToRadians(Params.SteeringData.Alpha0));
		const FVector RackEnd = RackStart - RackLen * AttachmentLeft.GetUnitAxis(EAxis::Y);

		const double KingpinLen = (AttachmentLeft.GetLocation() - KnuckleLeftPos).Length();
		const FVector SteeringArmStart = (RackStart + RackEnd) * 0.5;
		const FVector SteeringArmEnd = SteeringArmStart - AttachmentLeft.GetUnitAxis(EAxis::X) *
															  KingpinLen * Params.SteeringData.Lc;

		// Draw kingpins.
		PDI->DrawLine(
			AttachmentLeft.GetLocation(), KnuckleLeftPos, GetKingpinColor(), SDPG_Foreground, 2.f);
		PDI->DrawLine(
			AttachmentRight.GetLocation(), KnuckleRightPos, GetKingpinColor(), SDPG_Foreground,
			2.f);

		// Draw racks.
		PDI->DrawLine(KnuckleLeftPos, RackStart, GetRackColor(), SDPG_Foreground, 2.f);
		PDI->DrawLine(KnuckleRightPos, RackEnd, GetRackColor(), SDPG_Foreground, 2.f);

		// Draw center rack.
		PDI->DrawLine(RackStart, RackEnd, GetCentralRackColor(), SDPG_Foreground, 2.f);

		// Draw steering arm lines and spheres (the "triangle").
		PDI->DrawLine(
			SteeringArmStart, SteeringArmEnd, GetSteeringArmColor(), SDPG_Foreground, 2.f);
		DrawWireSphere(
			PDI, FTransform(FQuat::Identity, SteeringArmEnd), GetSphereColor(), 3.f, 8,
			SDPG_Foreground, 1.f);
		PDI->DrawLine(RackStart, SteeringArmEnd, GetSteeringArmColor(), SDPG_Foreground, 2.f);
		PDI->DrawLine(RackEnd, SteeringArmEnd, GetSteeringArmColor(), SDPG_Foreground, 2.f);
		DrawWireSphere(
			PDI, FTransform(FQuat::Identity, RackStart), GetSphereColor(), 3.f, 8, SDPG_Foreground,
			1.f);
		DrawWireSphere(
			PDI, FTransform(FQuat::Identity, RackEnd), GetSphereColor(), 3.f, 8, SDPG_Foreground,
			1.f);
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
