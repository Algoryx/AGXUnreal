// Copyright 2024, Algoryx Simulation AB.

#include "Constraints/AGX_ConstraintReferences.h"

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_HingeConstraintComponent.h"

FAGX_HingeReference::FAGX_HingeReference()
	: FAGX_ComponentReference(UAGX_HingeConstraintComponent::StaticClass())
{
}

UAGX_HingeConstraintComponent* FAGX_HingeReference::GetHinge() const
{
	return Super::GetComponent<UAGX_HingeConstraintComponent>();
}
