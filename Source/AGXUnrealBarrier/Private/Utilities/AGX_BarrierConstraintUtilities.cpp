// Copyright 2025, Algoryx Simulation AB.

#include "AGX_BarrierConstraintUtilities.h"

// AGX Dynamics for Unreal includes.
#include "TypeConversions.h"
#include "BarrierOnly/AGXRefs.h"
#include "AGX_AgxDynamicsObjectsAccess.h"

void FAGX_BarrierConstraintUtilities::ConvertConstraintBodiesAndFrames(
	const FRigidBodyBarrier& RigidBody1, const FVector& FramePosition1, const FQuat& FrameRotation1,
	const FRigidBodyBarrier* RigidBody2, const FVector& FramePosition2, const FQuat& FrameRotation2,
	agx::RigidBody*& OutNativeRigidBody1, agx::FrameRef& OutNativeFrame1,
	agx::RigidBody*& OutNativeRigidBody2, agx::FrameRef& OutNativeFrame2)
{
	// Convert first Rigid Body and Frame to natives. There must always be a first body. The Frame
	// created is relative to this body.
	OutNativeRigidBody1 = FAGX_AgxDynamicsObjectsAccess::GetFrom(RigidBody1);
	check(OutNativeRigidBody1);
	OutNativeFrame1 = ConvertFrame(FramePosition1, FrameRotation1);

	// The second rigid body is optional. If no body is given then the Frame created is relative
	// to the world.
	if (RigidBody2 != nullptr)
	{
		OutNativeRigidBody2 = FAGX_AgxDynamicsObjectsAccess::GetFrom(RigidBody2);
		check(OutNativeRigidBody2);
	}
	else
	{
		OutNativeRigidBody2 = nullptr;
	}
	OutNativeFrame2 = ConvertFrame(FramePosition2, FrameRotation2);
}

void FAGX_BarrierConstraintUtilities::ConvertConstraintBodyAndFrame(
	const FRigidBodyBarrier& Body, const FVector& FramePosition, const FQuat& FrameRotation,
	agx::RigidBody*& OutNativeBody, agx::FrameRef& OutNativeFrame)
{
	OutNativeBody = FAGX_AgxDynamicsObjectsAccess::GetFrom(Body);
	OutNativeFrame = ConvertFrame(FramePosition, FrameRotation);
}

agx::Angle::Type FAGX_BarrierConstraintUtilities::GetDofType(const agx::Motor1D* Motor)
{
	if (Motor == nullptr)
	{
		return InvalidAngleType;
	}
	const agx::Angle* Angle = Motor->getData().getAngle();
	if (Angle == nullptr)
	{
		return InvalidAngleType;
	}
	return Angle->getType();
}
