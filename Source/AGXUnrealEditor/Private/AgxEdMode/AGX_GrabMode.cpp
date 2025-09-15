// Copyright 2025, Algoryx Simulation AB.

#include "AgxEdMode/AGX_GrabMode.h"

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyComponent.h"
#include "AGX_Simulation.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"

// Unreal Engine includes.
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"
#include "EditorViewportClient.h"
#include "EditorModeManager.h"
#include "Framework/Application/SlateApplication.h"
#include "Kismet2/DebuggerCommands.h"
#include "Widgets/SToolTip.h"

const FEditorModeID FAGX_GrabMode::EM_AGX_GrabModeId = TEXT("EM_AGX_GrabModeId");

void FAGX_GrabMode::Activate()
{
	UWorld* World = FAGX_EditorUtilities::GetCurrentWorld();
	if (World == nullptr)
		return;

	if (!World->IsGameWorld())
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithError(
			"AGX Grab Mode can only be activated during play.");
		return;
	}

	if (!GLevelEditorModeTools().IsModeActive(EM_AGX_GrabModeId))
		GLevelEditorModeTools().ActivateMode(EM_AGX_GrabModeId);

	// Detach (eject) if attached.
	if (FPlayWorldCommandCallbacks::IsInPIE())
		GEditor->RequestToggleBetweenPIEandSIE();
}

void FAGX_GrabMode::Deactivate()
{
	if (GLevelEditorModeTools().IsModeActive(EM_AGX_GrabModeId))
		GLevelEditorModeTools().DeactivateMode(EM_AGX_GrabModeId);
}

namespace AGX_GrabMode_helpers
{
	UAGX_RigidBodyComponent* FindRigidBody(const UPrimitiveComponent& Component)
	{
		TArray<USceneComponent*> Ancestors;
		Component.GetParentComponents(Ancestors);

		for (USceneComponent* Ancestor : Ancestors)
		{
			if (auto Rb = Cast<UAGX_RigidBodyComponent>(Ancestor))
				return Rb;
		}

		return nullptr;
	}
}

void FAGX_GrabMode::OnMouseClickComponent(
	UPrimitiveComponent* Component, const FVector& WorldLocation,
	const FViewportCursorLocation& CursorInfo)
{
	Body.Reset();

	if (Component == nullptr)
		return;

	Body = AGX_GrabMode_helpers::FindRigidBody(*Component);
	if (Body == nullptr)
		return;

	ForceOriginLocalPos =
		Body->GetComponentTransform().InverseTransformPositionNoScale(WorldLocation);
	ForceOriginInitialDistance = (CursorInfo.GetOrigin() - WorldLocation).Length();

	if (LockConstraint.HasNative())
		DestroyLockConstraint();

	// Create Lock Constraint.
	LockConstraint.AllocateNative(
		*Body->GetNative(), ForceOriginLocalPos, FQuat::Identity, nullptr, WorldLocation,
		FQuat::Identity);
	LockConstraint.SetEnableComputeForces(true);
	LockConstraint.SetCompliance(1.0E-6, -1);

	auto Sim = UAGX_Simulation::GetFrom(Body.Get());
	Sim->GetNative()->Add(LockConstraint);
}

void FAGX_GrabMode::OnDeactivateMode()
{
	Deactivate();
}

void FAGX_GrabMode::OnMouseDrag(const FViewportCursorLocation& CursorInfo)
{
	if (!IsHoldingBody())
		return;

	const FVector ForceOriginWorld =
		Body->GetComponentTransform().TransformPositionNoScale(ForceOriginLocalPos);

	const FVector ForceEndPointWorld =
		CursorInfo.GetOrigin() + CursorInfo.GetDirection() * ForceOriginInitialDistance;

	DrawDebugLine(
		Body->GetWorld(), ForceOriginWorld, ForceEndPointWorld, FColor::Green, false, -1.f, 99,
		3.f);

	LockConstraint.SetLocalLocation(1, ForceEndPointWorld);

	FVector ConstraintForce, ConstraintTorque;
	LockConstraint.GetLastForce(Body->GetNative(), ConstraintForce, ConstraintTorque);
	Force = ConstraintForce.Length();

	UpdateCursorDecorator();
}

void FAGX_GrabMode::OnEndMouseDrag()
{
	Body.Reset();
	ForceOriginLocalPos = FVector::ZeroVector;
	ForceOriginInitialDistance = 0.0;
	DestroyCursorDecorator();
	DestroyLockConstraint();
}

bool FAGX_GrabMode::GetCursor(EMouseCursor::Type& OutCursor) const
{
	OutCursor = IsHoldingBody() ? EMouseCursor::GrabHandClosed : EMouseCursor::GrabHand;
	return true;
}

bool FAGX_GrabMode::IsHoldingBody() const
{
	return Body != nullptr && Body->HasNative();
}

FText FAGX_GrabMode::GetCursorDecoratorText() const
{
	return FText::FromString(FString::Printf(TEXT("%.2f N"), Force));
}

void FAGX_GrabMode::UpdateCursorDecorator()
{
	if (!CursorDecoratorWindow.IsValid())
	{
		CursorDecoratorWindow = SWindow::MakeCursorDecorator();
		FSlateApplication::Get().AddWindow(CursorDecoratorWindow.ToSharedRef(), true);
		CursorDecoratorWindow->SetContent(
			SNew(SToolTip).Text(this, &FAGX_GrabMode::GetCursorDecoratorText));
	}

	CursorDecoratorWindow->MoveWindowTo(
		FSlateApplication::Get().GetCursorPos() + FSlateApplication::Get().GetCursorSize());
}

void FAGX_GrabMode::DestroyCursorDecorator()
{
	if (CursorDecoratorWindow.IsValid())
	{
		CursorDecoratorWindow->RequestDestroyWindow();
		CursorDecoratorWindow.Reset();
	}
}

void FAGX_GrabMode::DestroyLockConstraint()
{
	if (!LockConstraint.HasNative())
		return;

	if (auto Sim = UAGX_Simulation::GetFrom(FAGX_EditorUtilities::GetCurrentWorld()))
		Sim->GetNative()->Remove(LockConstraint);

	LockConstraint.ReleaseNative();
}
