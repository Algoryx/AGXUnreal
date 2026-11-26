// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "ComponentVisualizer.h"

class AGXUNREALEDITOR_API FAGX_DistributedSpawnPointComponentVisualizer
	: public FComponentVisualizer
{
public:
	virtual void DrawVisualization(
		const UActorComponent* Component, const FSceneView* View,
		FPrimitiveDrawInterface* PDI) override;
};
