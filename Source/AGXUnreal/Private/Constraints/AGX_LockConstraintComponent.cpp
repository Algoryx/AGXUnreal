// Copyright 2021, Algoryx Simulation AB.


#include "Constraints/AGX_LockConstraintComponent.h"

#include "Constraints/LockJointBarrier.h"
#include "Utilities/AGX_ConstraintUtilities.h"

class FRigidBodyBarrier;

UAGX_LockConstraintComponent::UAGX_LockConstraintComponent()
	: UAGX_ConstraintComponent(
		  {EDofFlag::DofFlagTranslational1, EDofFlag::DofFlagTranslational2,
		   EDofFlag::DofFlagTranslational3, EDofFlag::DofFlagRotational1,
		   EDofFlag::DofFlagRotational2, EDofFlag::DofFlagRotational3})
{
	NativeBarrier.Reset(new FLockJointBarrier());
}

UAGX_LockConstraintComponent::~UAGX_LockConstraintComponent()
{
}

FLockJointBarrier* UAGX_LockConstraintComponent::GetNativeLock()
{
	return FAGX_ConstraintUtilities::GetNativeCast(this);
}

const FLockJointBarrier* UAGX_LockConstraintComponent::GetNativeLock() const
{
	return FAGX_ConstraintUtilities::GetNativeCast(this);
}

void UAGX_LockConstraintComponent::CreateNativeImpl()
{
	FAGX_ConstraintUtilities::CreateNative(
		NativeBarrier.Get(), BodyAttachment1, BodyAttachment2, GetFName(), GetOwner()->GetFName());
}
