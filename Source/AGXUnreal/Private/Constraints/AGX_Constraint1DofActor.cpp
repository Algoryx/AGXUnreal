// Copyright 2025, Algoryx Simulation AB.

#include "Constraints/AGX_Constraint1DofActor.h"

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_Constraint1DofComponent.h"

AAGX_Constraint1DofActor::AAGX_Constraint1DofActor()
{
}

AAGX_Constraint1DofActor::~AAGX_Constraint1DofActor()
{
}

UAGX_Constraint1DofComponent* AAGX_Constraint1DofActor::Get1DofComponent()
{
	return Cast<UAGX_Constraint1DofComponent>(ConstraintComponent);
}
