// Copyright 2025, Algoryx Simulation AB.

#include "Constraints/AGX_DistanceConstraintComponent.h"

// AGX Dynamics for Unreal includes.
#include "Constraints/DistanceJointBarrier.h"
#include "Utilities/AGX_ConstraintUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

class FRigidBodyBarrier;

UAGX_DistanceConstraintComponent::UAGX_DistanceConstraintComponent()
	: UAGX_Constraint1DofComponent(
		  TArray<EDofFlag> {
			  // All common DOFs are free.
		  })
{
	// The AGX Dynamics distance constraint need the Lock to be enabled to function.
	LockController.bEnable = true;

	NativeBarrier.Reset(new FDistanceJointBarrier());
}

UAGX_DistanceConstraintComponent::~UAGX_DistanceConstraintComponent()
{
}

FDistanceJointBarrier* UAGX_DistanceConstraintComponent::GetNativeDistance()
{
	return FAGX_ConstraintUtilities::GetNativeCast(this);
}

const FDistanceJointBarrier* UAGX_DistanceConstraintComponent::GetNativeDistance() const
{
	return FAGX_ConstraintUtilities::GetNativeCast(this);
}

void UAGX_DistanceConstraintComponent::AllocateNative()
{
	FAGX_ConstraintUtilities::CreateNative(
		NativeBarrier.Get(), BodyAttachment1, BodyAttachment2, GetFName(),
		GetLabelSafe(GetOwner()));
}
