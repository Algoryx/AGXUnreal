// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/AGX_ShovelHitProxies.h"

// AGX Dynamics for Unreal includes.
#include "Terrain/AGX_ShovelComponent.h"

IMPLEMENT_HIT_PROXY(HShovelHitProxy, HComponentVisProxy);

HShovelHitProxy::HShovelHitProxy(const UAGX_ShovelComponent* InShovel, EAGX_ShovelFrame InFrame)
	: HComponentVisProxy(InShovel)
	, Frame(InFrame)
{
}
