// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "ComponentVisualizer.h"

class FPrimitiveDrawInterface;
class FSceneView;
class UActorComponent;

/**
 * Component Visualizer of UAGX_ConstraintFrameComponent, which does the following:
 *
 * - Triggers visualization of related Constraint (see FAGX_ConstraintComponentVisualizer).
 *
 */
class AGXUNREALEDITOR_API FAGX_ConstraintFrameComponentVisualizer : public FComponentVisualizer
{
public:
	//~ Begin FComponentVisualizer Interface
	virtual void DrawVisualization(
		const UActorComponent* Component, const FSceneView* View,
		FPrimitiveDrawInterface* PDI) override;
	//~ End FComponentVisualizer Interface
};
