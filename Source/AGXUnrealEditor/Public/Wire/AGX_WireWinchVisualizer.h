// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Wire/AGX_WireEnums.h"

// Unreal Engine includes.
#include "ComponentVisualizer.h"

class UAGX_WireWinchComponent;

class AGXUNREALEDITOR_API FAGX_WireWinchVisualizer : public FComponentVisualizer
{
public:
	FAGX_WireWinchVisualizer();
	~FAGX_WireWinchVisualizer();

	//~ Begin FComponentVisualizer Interface.

	virtual void OnRegister() override;

	virtual void DrawVisualization(
		const UActorComponent* Component, const FSceneView* View,
		FPrimitiveDrawInterface* PDI) override;

	virtual bool VisProxyHandleClick(
		FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy,
		const FViewportClick& Click) override;

	virtual bool GetWidgetLocation(
		const FEditorViewportClient* ViewportClient, FVector& OutLocation) const override;

	virtual bool HandleInputDelta(
		FEditorViewportClient* ViewportClient, FViewport* Viewport, FVector& DeltaTranslate,
		FRotator& DeltaRotate, FVector& DeltaScale) override;

	virtual bool HandleInputKey(
		FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key,
		EInputEvent Event) override;

	virtual void EndEditing() override;

	//~ End FComponentVisualizer Interface.

	bool HasValidWinchSelection() const;
	UAGX_WireWinchComponent* GetSelectedWinch();
	const UAGX_WireWinchComponent* GetSelectedWinch() const;
	void ClearSelection();

private:
	EWinchSide SelectedWinchSide = EWinchSide::None;

	/**
	 * Property path from the owning Actor to the Wire Winch Component of the currently selected
	 * Wire Winch. We must use a path instead of a UAGX_WireComponent* because during Blueprint
	 * Reconstruction the Wire Component will be replaced by a new instance.
	 */
	FComponentPropertyPath WinchPropertyPath;
};
