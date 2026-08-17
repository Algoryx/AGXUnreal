// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "ComponentVisualizer.h"

class AGXUNREALEDITOR_API FAGX_WireLinkComponentVisualizer : public FComponentVisualizer
{
public:
	//~ Begin FComponentVisualizer Interface.
	virtual void DrawVisualization(
		const UActorComponent* Component, const FSceneView* View,
		FPrimitiveDrawInterface* PDI) override;
	//~ End FComponentVisualizer Interface.
};
