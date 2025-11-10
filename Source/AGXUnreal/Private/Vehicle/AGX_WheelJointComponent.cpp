// Copyright 2025, Algoryx Simulation AB.

#include "Vehicle/AGX_WheelJointComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Vehicle/WheelJointBarrier.h"
#include "Utilities/AGX_ConstraintUtilities.h"

struct FRigidBodyBarrier;

UAGX_WheelJointComponent::UAGX_WheelJointComponent()
	: UAGX_ConstraintComponent(
		  {EDofFlag::DofFlagTranslational1, EDofFlag::DofFlagTranslational2,
		   EDofFlag::DofFlagRotational1})
{
	NativeBarrier.Reset(new FWheelJointBarrier());
}

UAGX_WheelJointComponent::~UAGX_WheelJointComponent()
{
}

FWheelJointBarrier* UAGX_WheelJointComponent::GetNativeWheelJoint()
{
	return FAGX_ConstraintUtilities::GetNativeCast(this);
}

const FWheelJointBarrier* UAGX_WheelJointComponent::GetNativeWheelJoint() const
{
	return FAGX_ConstraintUtilities::GetNativeCast(this);
}

void UAGX_WheelJointComponent::CreateNativeImpl()
{
	FAGX_ConstraintUtilities::CreateNative(
		NativeBarrier.Get(), BodyAttachment1, BodyAttachment2, GetFName(),
		GetLabelSafe(GetOwner()));
}
