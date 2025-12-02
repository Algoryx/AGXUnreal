// Copyright 2025, Algoryx Simulation AB.

#include "Constraints/AGX_CylindricalConstraintActor.h"

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_CylindricalConstraintComponent.h"

AAGX_CylindricalConstraintActor::AAGX_CylindricalConstraintActor()
{
	SetConstraintComponent(
		CreateDefaultSubobject<UAGX_CylindricalConstraintComponent>(TEXT("Cylindrical")));
}

AAGX_CylindricalConstraintActor::~AAGX_CylindricalConstraintActor()
{
}
