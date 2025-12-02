// Copyright 2025, Algoryx Simulation AB.

#include "Wire/AGX_WireWinchVisualizer.h"

// AGX Dynamics for Unreal includes.
#include "Utilities/AGX_EditorUtilities.h"
#include "Wire/AGX_WireWinchComponent.h"
#include "Wire/AGX_WireHitProxies.h"

// Unreal Engine includes.
#include "Editor.h"
#include "SceneManagement.h"

FAGX_WireWinchVisualizer::FAGX_WireWinchVisualizer()
{
	// Create and register command list here, if we ever need one.
}

FAGX_WireWinchVisualizer::~FAGX_WireWinchVisualizer()
{
	// Unregister command list here, if we ever need one.
}

void FAGX_WireWinchVisualizer::OnRegister()
{
	// Map command list here, if we ever need one.
}

void FAGX_WireWinchVisualizer::DrawVisualization(
	const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UAGX_WireWinchComponent* WinchComponent = Cast<UAGX_WireWinchComponent>(Component);
	if (WinchComponent == nullptr)
	{
		return;
	}

	const bool bSelected = FAGX_EditorUtilities::IsSelected(*WinchComponent);
	AGX_WireVisualization_helpers::DrawWinch(*WinchComponent, bSelected, PDI);
}

bool FAGX_WireWinchVisualizer::VisProxyHandleClick(
	FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy,
	const FViewportClick& Click)
{
	const UAGX_WireWinchComponent* Winch = Cast<const UAGX_WireWinchComponent>(VisProxy->Component);
	if (Winch == nullptr)
	{
		ClearSelection();
		return false;
	}

	AActor* OldOwningActor = WinchPropertyPath.GetParentOwningActor();
	AActor* NewOwningActor = Winch->GetOwner();
	if (NewOwningActor != OldOwningActor)
	{
		ClearSelection();
	}

	if (HWinchLocationProxy* LocationProxy = HitProxyCast<HWinchLocationProxy>(VisProxy))
	{
		if (SelectedWinchSide == EWinchSide::Location)
		{
			// Clicking a selected handle deselects it.
			ClearSelection();
		}
		else
		{
			SelectedWinchSide = EWinchSide::Location;
			WinchPropertyPath = FComponentPropertyPath(Winch);
		}
		return true;
	}
	else if (HWinchDirectionProxy* DirectionProxy = HitProxyCast<HWinchDirectionProxy>(VisProxy))
	{
		if (SelectedWinchSide == EWinchSide::Rotation)
		{
			// Clicking a selected handle deselects it.
			ClearSelection();
		}
		else
		{
			SelectedWinchSide = EWinchSide::Rotation;
			WinchPropertyPath = FComponentPropertyPath(Winch);
		}
		return true;
	}

	return false;
}

bool FAGX_WireWinchVisualizer::GetWidgetLocation(
	const FEditorViewportClient* ViewportClient, FVector& OutLocation) const
{
	if (!HasValidWinchSelection())
	{
		return false;
	}

	const UAGX_WireWinchComponent* SelectedWinch = GetSelectedWinch();
	checkf(
		SelectedWinch != nullptr,
		TEXT("HasValidWinchSelection has been checked but we still didn't get a winch."));

	FAGX_WireWinchPose WinchPose = FAGX_WireUtilities::GetWireWinchPose(*GetSelectedWinch());
	return AGX_WireVisualization_helpers::GetWidgetLocation(
		WinchPose, SelectedWinchSide, OutLocation);
}

bool FAGX_WireWinchVisualizer::HandleInputDelta(
	FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate,
	FRotator& DeltaRotate, FVector& DeltaScale)
{
	using namespace AGX_WireVisualization_helpers;

	if (!HasValidWinchSelection())
	{
		return false;
	}

	UAGX_WireWinchComponent* WinchComponent = GetSelectedWinch();
	const FTransform& WinchToWorld = FAGX_WireUtilities::GetWinchLocalToWorld(*WinchComponent);
	FAGX_WireWinch& Winch = WinchComponent->WireWinch;

	AGX_WireVisualization_helpers::TransformWinch(
		Winch, WinchToWorld, SelectedWinchSide, DeltaTranslate, DeltaRotate);

	GEditor->RedrawLevelEditingViewports();
	return true;
}

bool FAGX_WireWinchVisualizer::HandleInputKey(
	FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
	return false;
}

void FAGX_WireWinchVisualizer::EndEditing()
{
	ClearSelection();
}

bool FAGX_WireWinchVisualizer::HasValidWinchSelection() const
{
	return GetSelectedWinch() != nullptr && SelectedWinchSide != EWinchSide::None;
}

UAGX_WireWinchComponent* FAGX_WireWinchVisualizer::GetSelectedWinch()
{
	return Cast<UAGX_WireWinchComponent>(WinchPropertyPath.GetComponent());
}

const UAGX_WireWinchComponent* FAGX_WireWinchVisualizer::GetSelectedWinch() const
{
	return Cast<const UAGX_WireWinchComponent>(WinchPropertyPath.GetComponent());
}

void FAGX_WireWinchVisualizer::ClearSelection()
{
	SelectedWinchSide = EWinchSide::None;
	WinchPropertyPath.Reset();
}
