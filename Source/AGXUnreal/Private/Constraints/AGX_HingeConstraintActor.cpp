// Copyright 2025, Algoryx Simulation AB.

#include "Constraints/AGX_HingeConstraintActor.h"

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_HingeConstraintComponent.h"

AAGX_HingeConstraintActor::AAGX_HingeConstraintActor()
{
	SetConstraintComponent(CreateDefaultSubobject<UAGX_HingeConstraintComponent>(TEXT("Hinge")));
}

AAGX_HingeConstraintActor::~AAGX_HingeConstraintActor()
{
}
