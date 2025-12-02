// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "ComponentVisualizer.h"

class FPrimitiveDrawInterface;
class FSceneView;
class UActorComponent;

/**
 * Component Visualizer of UAGX_TireComponent, which draws some simple primitives to visualize the
 * tire model.
 */
class AGXUNREALEDITOR_API FAGX_TireComponentVisualizer : public FComponentVisualizer
{
public:
	virtual void DrawVisualization(
		const UActorComponent* Component, const FSceneView* View,
		FPrimitiveDrawInterface* PDI) override;
};
