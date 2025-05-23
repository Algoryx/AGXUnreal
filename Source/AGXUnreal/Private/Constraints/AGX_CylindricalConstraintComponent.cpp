// Copyright 2025, Algoryx Simulation AB.

#include "Constraints/AGX_CylindricalConstraintComponent.h"

// AGX Dynamics for Unreal includes.
#include "Constraints/CylindricalJointBarrier.h"
#include "Utilities/AGX_ConstraintUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

class FRigidBodyBarrier;

UAGX_CylindricalConstraintComponent::UAGX_CylindricalConstraintComponent()
	: UAGX_Constraint2DofComponent(
		  {EDofFlag::DofFlagRotational1, EDofFlag::DofFlagRotational2,
		   EDofFlag::DofFlagTranslational1, EDofFlag::DofFlagTranslational2})
{
	NativeBarrier.Reset(new FCylindricalJointBarrier());
}

UAGX_CylindricalConstraintComponent::~UAGX_CylindricalConstraintComponent()
{
}

FCylindricalJointBarrier* UAGX_CylindricalConstraintComponent::GetNativeCylindrical()
{
	return FAGX_ConstraintUtilities::GetNativeCast(this);
}

const FCylindricalJointBarrier* UAGX_CylindricalConstraintComponent::GetNativeCylindrical() const
{
	return FAGX_ConstraintUtilities::GetNativeCast(this);
}

void UAGX_CylindricalConstraintComponent::AllocateNative()
{
	FAGX_ConstraintUtilities::CreateNative(
		NativeBarrier.Get(), BodyAttachment1, BodyAttachment2, GetFName(),
		GetLabelSafe(GetOwner()));
}
