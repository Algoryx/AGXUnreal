// Copyright 2025, Algoryx Simulation AB.

#include "AGX_SceneComponentReference.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"

FAGX_SceneComponentReference::FAGX_SceneComponentReference()
	: FAGX_ComponentReference(USceneComponent::StaticClass())
{
}

USceneComponent* FAGX_SceneComponentReference::GetSceneComponent() const
{
	return Super::GetComponent<USceneComponent>();
}
