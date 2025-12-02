// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

class UAGX_ShovelComponent;

class AGXUNREAL_API FAGX_ShovelUtilities
{
public:
	// The size of Hit Proxies created by the various Shovel-related Visualizers we have.
	// This may become a configurable setting in the future.
	static constexpr float HitProxySize = 10.0f; // todo Where should this value live?
};
