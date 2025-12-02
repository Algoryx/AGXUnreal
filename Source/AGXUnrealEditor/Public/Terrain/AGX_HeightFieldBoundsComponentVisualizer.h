// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Unreal includes.
#include "CoreMinimal.h"
#include "ComponentVisualizer.h"
#include "IDetailCustomization.h"

class AGXUNREALEDITOR_API FAGX_HeightFieldBoundsComponentVisualizer : public FComponentVisualizer
{
public:
	//~ Begin FComponentVisualizer Interface
	virtual void DrawVisualization(
		const UActorComponent* Component, const FSceneView* View,
		FPrimitiveDrawInterface* PDI) override;
	//~ End FComponentVisualizer Interface
};
