// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "Constraints/ConstraintBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Constraints/TwistRangeControllerBarrier.h"

struct FRigidBodyBarrier;

class AGXUNREALBARRIER_API FBallJointBarrier : public FConstraintBarrier
{
public: // Special member functions.
	FBallJointBarrier();
	FBallJointBarrier(FBallJointBarrier&& Other) = default;
	FBallJointBarrier(std::unique_ptr<FConstraintRef> Native);
	virtual ~FBallJointBarrier();

public: // Accessor functions.
	FTwistRangeControllerBarrier GetTwistRangeController() const;

private: // Native management.
	virtual void AllocateNativeImpl(
		const FRigidBodyBarrier& Rb1, const FVector& FramePosition1, const FQuat& FrameRotation1,
		const FRigidBodyBarrier* Rb2, const FVector& FramePosition2,
		const FQuat& FrameRotation2) override;

private: // Deleted functions.
	FBallJointBarrier(const FBallJointBarrier&) = delete;
	void operator=(const FBallJointBarrier&) = delete;
};
