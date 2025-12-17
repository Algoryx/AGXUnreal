// Copyright 2025, Algoryx Simulation AB.

#include "Vehicle/AGX_WheelJointReference.h"

// AGX Dynamics for Unreal includes.
#include "Vehicle/AGX_WheelJointComponent.h"

FAGX_WheelJointReference::FAGX_WheelJointReference()
	: FAGX_ComponentReference(UAGX_WheelJointComponent::StaticClass())
{
}

UAGX_WheelJointComponent* FAGX_WheelJointReference::GetWheelJointComponent() const
{
	return Super::GetComponent<UAGX_WheelJointComponent>();
}

void UAGX_WheelJointReference_FL::SetWheelJointComponent(
	FAGX_WheelJointReference& Reference, UAGX_WheelJointComponent* Component)
{
	Reference.SetComponent(Component);
}

UAGX_WheelJointComponent* UAGX_WheelJointReference_FL::GetWheelJointComponent(
	FAGX_WheelJointReference& Reference)
{
	return Cast<UAGX_WheelJointComponent>(Reference.GetComponent());
}
