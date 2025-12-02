// Copyright 2025, Algoryx Simulation AB.

#include "Constraints/AGX_LockConstraintActor.h"

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_LockConstraintComponent.h"

AAGX_LockConstraintActor::AAGX_LockConstraintActor()
{
	SetConstraintComponent(CreateDefaultSubobject<UAGX_LockConstraintComponent>(TEXT("Lock")));
}

AAGX_LockConstraintActor::~AAGX_LockConstraintActor()
{
}
