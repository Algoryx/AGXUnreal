// Copyright 2025, Algoryx Simulation AB.

#include "Sensors/AGX_LidarSensorReference.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarSensorComponent.h"

FAGX_LidarSensorReference::FAGX_LidarSensorReference()
	: FAGX_ComponentReference(UAGX_LidarSensorComponent::StaticClass())
{
}

UAGX_LidarSensorComponent* FAGX_LidarSensorReference::GetLidarComponent() const
{
	return Super::GetComponent<UAGX_LidarSensorComponent>();
}


// Blueprint API

void UAGX_LidarSensorReference_FL::SetLidarComponent(
	FAGX_LidarSensorReference& Reference, UAGX_LidarSensorComponent* Component)
{
	Reference.SetComponent(Component);
}

UAGX_LidarSensorComponent* UAGX_LidarSensorReference_FL::GetLidarComponent(
	FAGX_LidarSensorReference& Reference)
{
	return Cast<UAGX_LidarSensorComponent>(Reference.GetComponent());
}
