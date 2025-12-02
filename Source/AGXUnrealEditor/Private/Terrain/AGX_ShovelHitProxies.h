// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Terrain/AGX_TerrainEnums.h"

// Unreal Engine includes.
#include "ComponentVisualizer.h"

class UAGX_ShovelComponent;

class HShovelHitProxy : public HComponentVisProxy
{
	DECLARE_HIT_PROXY();

	HShovelHitProxy(const UAGX_ShovelComponent* InShovel, EAGX_ShovelFrame InFrame);

	EAGX_ShovelFrame Frame;
};
