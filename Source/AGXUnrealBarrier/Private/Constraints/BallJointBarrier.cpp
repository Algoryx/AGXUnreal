// Fill out your copyright notice in the Description page of Project Settings.


#include "Constraints/BallJointBarrier.h"

#include "AGXRefs.h"
#include "RigidBodyBarrier.h"
#include "TypeConversions.h"

#include "BeginAGXIncludes.h"
#include <agx/BallJoint.h>
#include "EndAGXIncludes.h"


FBallJointBarrier::FBallJointBarrier()
	: FConstraintBarrier()
{
}

FBallJointBarrier::~FBallJointBarrier()
{
}

void FBallJointBarrier::AllocateNativeImpl(
	const FRigidBodyBarrier *RigidBody1, const FVector *FramePosition1, const FQuat *FrameRotation1,
	const FRigidBodyBarrier *RigidBody2, const FVector *FramePosition2, const FQuat *FrameRotation2)
{
	check(!HasNative());

	agx::RigidBody* NativeRigidBody1 = nullptr;
	agx::RigidBody* NativeRigidBody2 = nullptr;
	agx::FrameRef NativeFrame1 = nullptr;
	agx::FrameRef NativeFrame2 = nullptr;

	ConvertConstraintBodiesAndFrames(
		RigidBody1, FramePosition1, FrameRotation1,
		RigidBody2, FramePosition2, FrameRotation2,
		NativeRigidBody1, NativeFrame1,
		NativeRigidBody2, NativeFrame2);

	NativeRef->Native = new agx::BallJoint(
		NativeRigidBody1, NativeFrame1.get(),
		NativeRigidBody2, NativeFrame2.get());
}