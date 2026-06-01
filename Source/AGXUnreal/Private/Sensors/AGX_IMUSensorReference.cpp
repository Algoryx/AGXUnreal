// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_IMUSensorReference.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_IMUSensorComponent.h"

FAGX_IMUSensorReference::FAGX_IMUSensorReference()
	: FAGX_ComponentReference(UAGX_IMUSensorComponent::StaticClass())
{
}

UAGX_IMUSensorComponent* FAGX_IMUSensorReference::GetIMUComponent() const
{
	return Super::GetComponent<UAGX_IMUSensorComponent>();
}

// Blueprint API

void UAGX_IMUSensorReference_FL::SetIMUComponent(
	FAGX_IMUSensorReference& Reference, UAGX_IMUSensorComponent* Component)
{
	Reference.SetComponent(Component);
}

UAGX_IMUSensorComponent* UAGX_IMUSensorReference_FL::GetIMUComponent(
	FAGX_IMUSensorReference& Reference)
{
	return Cast<UAGX_IMUSensorComponent>(Reference.GetComponent());
}
