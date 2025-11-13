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
