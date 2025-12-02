// Copyright 2025, Algoryx Simulation AB.

#include "Constraints/AGX_PrismaticConstraintActor.h"

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_PrismaticConstraintComponent.h"

AAGX_PrismaticConstraintActor::AAGX_PrismaticConstraintActor()
{
	SetConstraintComponent(
		CreateDefaultSubobject<UAGX_PrismaticConstraintComponent>(TEXT("Prismatic")));
}

AAGX_PrismaticConstraintActor::~AAGX_PrismaticConstraintActor()
{
}
