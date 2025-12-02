// Copyright 2025, Algoryx Simulation AB.

#include "AgxEdMode/AGX_ClickDragMode.h"

// Unreal Engine includes.
#include "CollisionQueryParams.h"
#include "EditorModes.h"
#include "Engine/HitResult.h"
#include "LevelEditorViewport.h"

namespace AGX_ClickDragMode_helpers
{
	void GetViewportMouseRayStartAndEnd(
		FEditorViewportClient& ViewportClient, FVector& OutStart, FVector& OutEnd)
	{
		OutStart = OutEnd = FVector::ZeroVector;

		const FViewportCursorLocation MouseViewportRay =
			ViewportClient.GetCursorWorldLocationFromMousePos();
		const FVector& MouseRayOrigin = MouseViewportRay.GetOrigin();
		const FVector& MouseRayDirection = MouseViewportRay.GetDirection();

		OutStart = MouseRayOrigin;
		OutEnd = OutStart + WORLD_MAX * MouseRayDirection;
	}
}

bool FAGX_ClickDragMode::InputKey(
	FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	using namespace AGX_ClickDragMode_helpers;

	SetClickDragInactive();

	if (ViewportClient == nullptr || ViewportClient != GCurrentLevelEditingViewportClient)
	{
		OnDeactivateMode();
		return false; // Not sure if this should ever happen.
	}

	if (Key == EKeys::Escape && Event == IE_Pressed)
	{
		OnDeactivateMode();
		return true; // Consume input.
	}

	if (Key != EKeys::LeftMouseButton || Event != IE_Pressed)
		return false; // Allow input to be handled elsewhere, e.g. for moving camera around.

	FVector RayStart, RayEnd;
	GetViewportMouseRayStartAndEnd(*ViewportClient, RayStart, RayEnd);

	// Line trace.
	UWorld* World = ViewportClient->GetWorld();
	FCollisionQueryParams CollParams;
	CollParams.bTraceComplex = true;
	FHitResult HitResult;
	if (!World->LineTraceSingleByChannel(HitResult, RayStart, RayEnd, ECC_Visibility, CollParams))
		return true; // Consume input.

	ClickDragState = EAGX_ClickDragState::Click;
	OnMouseClickComponent(
		HitResult.GetComponent(), HitResult.Location,
		ViewportClient->GetCursorWorldLocationFromMousePos());
	return true;
}

bool FAGX_ClickDragMode::CapturedMouseMove(
	FEditorViewportClient* InViewportClient, FViewport* InViewport, int32 InMouseX, int32 InMouseY)
{
	if (InViewportClient == nullptr || InViewportClient != GCurrentLevelEditingViewportClient)
	{
		OnDeactivateMode();
		return false; // Not sure if this should ever happen.
	}

	if (ClickDragState == EAGX_ClickDragState::Click)
		ClickDragState = EAGX_ClickDragState::Drag;

	return true;
}

bool FAGX_ClickDragMode::GetCursor(EMouseCursor::Type& OutCursor) const
{
	OutCursor = EMouseCursor::Crosshairs;
	return true;
}

bool FAGX_ClickDragMode::IsCompatibleWith(FEditorModeID OtherModeID) const
{
	// We want to be able to perform this action with all the built-in editor modes
	return OtherModeID != FBuiltinEditorModes::EM_None;
}

void FAGX_ClickDragMode::Tick(FEditorViewportClient* ViewportClient, float DeltaTime)
{
	if (ClickDragState == EAGX_ClickDragState::Drag)
		OnMouseDrag(ViewportClient->GetCursorWorldLocationFromMousePos());

	FEdMode::Tick(ViewportClient, DeltaTime);
}

bool FAGX_ClickDragMode::LostFocus(FEditorViewportClient* ViewportClient, FViewport* Viewport)
{
	SetClickDragInactive();
	OnDeactivateMode();
	return false;
}

void FAGX_ClickDragMode::SetClickDragInactive()
{
	if (ClickDragState == EAGX_ClickDragState::Drag)
		OnEndMouseDrag();

	ClickDragState = EAGX_ClickDragState::Inactive;
}
