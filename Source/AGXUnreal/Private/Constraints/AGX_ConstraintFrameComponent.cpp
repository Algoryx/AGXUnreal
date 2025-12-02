// Copyright 2025, Algoryx Simulation AB.

#include "Constraints/AGX_ConstraintFrameComponent.h"

UAGX_ConstraintFrameComponent::UAGX_ConstraintFrameComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAGX_ConstraintFrameComponent::AddConstraintUsage(UAGX_ConstraintComponent* Constraint)
{
	UsedByConstraints.AddUnique(Constraint);
}

void UAGX_ConstraintFrameComponent::RemoveConstraintUsage(UAGX_ConstraintComponent* Constraint)
{
	// Only remove first occurance, because it is actually valid for a constraint
	// to use the same Constraint Frame Actor twice.
	UsedByConstraints.RemoveSingle(Constraint);
}

const TArray<UAGX_ConstraintComponent*>& UAGX_ConstraintFrameComponent::GetConstraintUsage() const
{
	return UsedByConstraints;
}
