// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Unreal includes.
#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "ConstraintComponentVisualizer.h"

class UAGX_ConstraintComponent;
class FPrimitiveDrawInterface;
class FSceneView;
class UActorComponent;

/**
 * Component Visualizer of UAGX_ConstraintComponent, which does the following:
 *
 * - Highlights involved Rigid Body Actors.
 * - Draw tripods for the final Attachment Frames.
 *
 */
class AGXUNREALEDITOR_API FAGX_ConstraintComponentVisualizer : public FComponentVisualizer
{
public:
	//~ Begin FComponentVisualizer Interface

	virtual void DrawVisualization(
		const UActorComponent* Component, const FSceneView* View,
		FPrimitiveDrawInterface* PDI) override;

	virtual void DrawVisualizationHUD(
		const UActorComponent* Component, const FViewport* Viewport, const FSceneView* View,
		FCanvas* Canvas) override;

	virtual bool VisProxyHandleClick(
		FEditorViewportClient* InViewportClient, HComponentVisProxy* VisProxy,
		const FViewportClick& Click) override;

	//~ End FComponentVisualizer Interface

	static void DrawConstraint(
		const UAGX_ConstraintComponent* Constraint, const FSceneView* View,
		FPrimitiveDrawInterface* PDI);
	static void DrawConstraintHUD(
		const UAGX_ConstraintComponent* Constraint, const FViewport* Viewport,
		const FSceneView* View, FCanvas* Canvas);
};
