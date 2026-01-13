// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Cable/AGX_CableComponent.h"

// Unreal Engine includes.
#include "ComponentVisualizer.h"

/**
 * Data associated with clickable node visualization elements.
 */
class HAGX_CableNodeProxy : public HComponentVisProxy
{
	DECLARE_HIT_PROXY();

	HAGX_CableNodeProxy(const UAGX_CableComponent* InCable, int32 InNodeIndex)
		: HComponentVisProxy(InCable, HPP_Wireframe)
		, NodeIndex(InNodeIndex)
	{
	}

	// The index of the node that the visualization that this HAGX_CableNodeProxy is bound to
	// represents.
	int32 NodeIndex;
};
