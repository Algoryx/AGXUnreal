// Copyright 2025, Algoryx Simulation AB.

#include "Cable/AGX_CableRouteNode.h"

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyComponent.h"

void FAGX_CableRouteNode::SetBody(UAGX_RigidBodyComponent* Body)
{
	RigidBody.SetComponent(Body);
}
