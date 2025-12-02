// Copyright 2025, Algoryx Simulation AB.

#include "Constraints/CylindricalJointBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"
#include "RigidBodyBarrier.h"
#include "Utilities/AGX_BarrierConstraintUtilities.h"

#include "BeginAGXIncludes.h"
#include <agx/CylindricalJoint.h>
#include "EndAGXIncludes.h"

FCylindricalJointBarrier::FCylindricalJointBarrier()
	: FConstraint2DOFBarrier()
{
}

FCylindricalJointBarrier::FCylindricalJointBarrier(std::unique_ptr<FConstraintRef> Native)
	: FConstraint2DOFBarrier(std::move(Native))
{
}

FCylindricalJointBarrier::~FCylindricalJointBarrier()
{
}

void FCylindricalJointBarrier::AllocateNativeImpl(
	const FRigidBodyBarrier& RigidBody1, const FVector& FramePosition1, const FQuat& FrameRotation1,
	const FRigidBodyBarrier* RigidBody2, const FVector& FramePosition2, const FQuat& FrameRotation2)
{
	check(!HasNative());

	agx::RigidBody* NativeRigidBody1 = nullptr;
	agx::RigidBody* NativeRigidBody2 = nullptr;
	agx::FrameRef NativeFrame1 = nullptr;
	agx::FrameRef NativeFrame2 = nullptr;

	FAGX_BarrierConstraintUtilities::ConvertConstraintBodiesAndFrames(
		RigidBody1, FramePosition1, FrameRotation1, RigidBody2, FramePosition2, FrameRotation2,
		NativeRigidBody1, NativeFrame1, NativeRigidBody2, NativeFrame2);

	NativeRef->Native = new agx::CylindricalJoint(
		NativeRigidBody1, NativeFrame1.get(), NativeRigidBody2, NativeFrame2.get());
}
