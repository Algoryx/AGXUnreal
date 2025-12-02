// Copyright 2025, Algoryx Simulation AB.

#include "Constraints/HingeBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"
#include "RigidBodyBarrier.h"
#include "Utilities/AGX_BarrierConstraintUtilities.h"

#include "BeginAGXIncludes.h"
#include <agx/Hinge.h>
#include "EndAGXIncludes.h"

FHingeBarrier::FHingeBarrier()
	: FConstraint1DOFBarrier()
{
}

FHingeBarrier::FHingeBarrier(std::unique_ptr<FConstraintRef> Native)
	: FConstraint1DOFBarrier(std::move(Native))
{
	/// \todo Should we allow nullptr hinge here?
	check(NativeRef->Native->is<agx::Hinge>());
}

FHingeBarrier::~FHingeBarrier()
{
}

void FHingeBarrier::AllocateNativeImpl(
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

	NativeRef->Native =
		new agx::Hinge(NativeRigidBody1, NativeFrame1.get(), NativeRigidBody2, NativeFrame2.get());
}
