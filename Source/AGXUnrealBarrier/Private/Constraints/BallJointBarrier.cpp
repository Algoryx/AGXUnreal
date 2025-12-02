// Copyright 2025, Algoryx Simulation AB.

#include "Constraints/BallJointBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGXBarrierFactories.h"
#include "BarrierOnly/AGXRefs.h"
#include "RigidBodyBarrier.h"
#include "Utilities/AGX_BarrierConstraintUtilities.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/BallJoint.h>
#include "EndAGXIncludes.h"

namespace
{
	agx::BallJoint* GetNative(FBallJointBarrier& Barrier)
	{
		return Barrier.GetNative()->Native->as<agx::BallJoint>();
	}

	const agx::BallJoint* GetNative(const FBallJointBarrier& Barrier)
	{
		return Barrier.GetNative()->Native->as<agx::BallJoint>();
	}
}

FBallJointBarrier::FBallJointBarrier()
	: FConstraintBarrier()
{
}

FBallJointBarrier::FBallJointBarrier(std::unique_ptr<FConstraintRef> Native)
	: FConstraintBarrier(std::move(Native))
{
	if (HasNative())
	{
		check(NativeRef->Native->is<agx::BallJoint>());
	}
}

FBallJointBarrier::~FBallJointBarrier()
{
}

FTwistRangeControllerBarrier FBallJointBarrier::GetTwistRangeController() const
{
	check(HasNative());
	agx::TwistRangeController* Controller = ::GetNative(*this)->getTwistRangeController();
	return AGXBarrierFactories::CreateTwistRangeControllerBarrier(Controller);
}

void FBallJointBarrier::AllocateNativeImpl(
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

	NativeRef->Native = new agx::BallJoint(
		NativeRigidBody1, NativeFrame1.get(), NativeRigidBody2, NativeFrame2.get());
}
