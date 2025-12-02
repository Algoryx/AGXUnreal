// Copyright 2025, Algoryx Simulation AB.

#include "Constraints/AGX_BallConstraintActor.h"

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_BallConstraintComponent.h"

AAGX_BallConstraintActor::AAGX_BallConstraintActor()
{
	SetConstraintComponent(CreateDefaultSubobject<UAGX_BallConstraintComponent>(TEXT("Ball")));
}

AAGX_BallConstraintActor::~AAGX_BallConstraintActor()
{
}
