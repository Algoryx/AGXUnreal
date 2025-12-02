// Copyright 2025, Algoryx Simulation AB.

#include "Constraints/AnyConstraintBarrier.h"

// AGX Dynamics for Unreal includes.
#include <AGX_LogCategory.h>
#include "BarrierOnly/AGXRefs.h"
#include "RigidBodyBarrier.h"

FAnyConstraintBarrier::FAnyConstraintBarrier()
	: FConstraintBarrier()
{
}

FAnyConstraintBarrier::FAnyConstraintBarrier(std::unique_ptr<FConstraintRef> Native)
	: FConstraintBarrier(std::move(Native))
{
}

FAnyConstraintBarrier::FAnyConstraintBarrier(FAnyConstraintBarrier&& Other)
	: FConstraintBarrier(std::move(Other))
{
}

FAnyConstraintBarrier::~FAnyConstraintBarrier()
{
}

void FAnyConstraintBarrier::AllocateNativeImpl(
	const FRigidBodyBarrier& /*RigidBody1*/, const FVector& /*FramePosition1*/,
	const FQuat& /*FrameRotation1*/, const FRigidBodyBarrier* /*RigidBody2*/,
	const FVector& /*FramePosition2*/, const FQuat& /*FrameRotation2*/)
{
	UE_LOG(
		LogAGX, Error,
		TEXT("AllocateNativeImpl called on an FAnyConstraintBarrier. This is not supported because "
			 "this type doesn't know what type of AGX Dynamics shape to allocate."))
}
