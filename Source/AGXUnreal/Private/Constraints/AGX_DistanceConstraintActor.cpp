// Copyright 2025, Algoryx Simulation AB.

#include "Constraints/AGX_DistanceConstraintActor.h"

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_DistanceConstraintComponent.h"

AAGX_DistanceConstraintActor::AAGX_DistanceConstraintActor()
{
	SetConstraintComponent(
		CreateDefaultSubobject<UAGX_DistanceConstraintComponent>(TEXT("Distance")));
}

AAGX_DistanceConstraintActor::~AAGX_DistanceConstraintActor()
{
}
