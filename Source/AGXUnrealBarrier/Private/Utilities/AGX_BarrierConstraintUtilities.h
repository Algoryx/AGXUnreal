// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "RigidBodyBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Math/Quat.h"

// AGX Dynamics includes
#include "BeginAGXIncludes.h"
#include <agx/Constraint.h>
#include <agx/ConstraintAngle.h>
#include <agx/RigidBody.h>
#include "EndAGXIncludes.h"

class AGXUNREALBARRIER_API FAGX_BarrierConstraintUtilities
{
public:
	/**
	 * Convert a constraint attachment pair to the native AGX Dynamics representation.
	 *
	 * The two position/rotation pairs are converted to the corresponding agx::Frames.
	 * The two bodies are returned as agx::RigidBodies, if there is one.
	 * The first body MUST have an agx::RigidBody.
	 *
	 * No return value, the result is stored to the out parameters.
	 *
	 * @param RigidBody1 The first body. Must have an agx::RigidBody.
	 * @param FramePosition1 The position of the first attachment in the body's local space.
	 * @param FrameRotation1 The rotation of the first attachment in the body's local space.
	 * @param RigidBody2 Second body. May be nullptr. If not then it must have an agx::RigidBody.
	 * @param FramePosition2 Position of second attachment, either in body or world space.
	 * @param FrameRotation2 Rotation of second attachment, either in body or world space.
	 * @param OutNativeRigidBody1 Output parameter for the first agx::RigidBody.
	 * @param OutNativeFrame1 Output parameter for the first agx::Frame.
	 * @param OutNativeRigidBody2 Output parameter for the second agx::RigidBody.
	 * @param OutNativeFrame2 Output parameter for the second agx::Frame.
	 */
	static void ConvertConstraintBodiesAndFrames(
		const FRigidBodyBarrier& RigidBody1, const FVector& FramePosition1,
		const FQuat& FrameRotation1, const FRigidBodyBarrier* RigidBody2,
		const FVector& FramePosition2, const FQuat& FrameRotation2,
		agx::RigidBody*& OutNativeRigidBody1, agx::FrameRef& OutNativeFrame1,
		agx::RigidBody*& OutNativeRigidBody2, agx::FrameRef& OutNativeFrame2);

	/**
	 * Convert a constraint attachment to the native AGX Dynamics representation.
	 *
	 * This overload should be used if you do not want a second constraint frame, which has the
	 * effect of constraining the body where it is right now. If you have two bodies or two
	 * attachments then use the other overload.
	 */
	static void ConvertConstraintBodyAndFrame(
		const FRigidBodyBarrier& Body, const FVector& FramePosition, const FQuat& FrameRotation,
		agx::RigidBody*& OutNativeBody, agx::FrameRef& OutNativeFrame);

	// Let's hope -1 is never used for a valid angle type.
	// TODO: Find a better way than forcing a made-up enum value into an agx::Angle::Type.
	static /*constexpr*/ const agx::Angle::Type InvalidAngleType =
		static_cast<agx::Angle::Type>(-1);

	static agx::Angle::Type GetDofType(const agx::Motor1D* Motor);
};
