// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/AGX_ShovelReference.h"

// AGX Dynamics for Unreal includes.
#include "Terrain/AGX_ShovelComponent.h"

FAGX_ShovelReference::FAGX_ShovelReference()
	: FAGX_ComponentReference(UAGX_ShovelComponent::StaticClass())
{
}

UAGX_ShovelComponent* FAGX_ShovelReference::GetShovelComponent() const
{
	return Super::GetComponent<UAGX_ShovelComponent>();
}
