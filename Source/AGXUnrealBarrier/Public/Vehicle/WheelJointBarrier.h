// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "Constraints/ConstraintBarrier.h"

struct FRigidBodyBarrier;

class AGXUNREALBARRIER_API FWheelJointBarrier : public FConstraintBarrier
{
public:
	FWheelJointBarrier();
	FWheelJointBarrier(FWheelJointBarrier&& Other) = default;
	FWheelJointBarrier(std::unique_ptr<FConstraintRef> Native);
	virtual ~FWheelJointBarrier();

private:
	virtual void AllocateNativeImpl(
		const FRigidBodyBarrier& Rb1, const FVector& FramePosition1, const FQuat& FrameRotation1,
		const FRigidBodyBarrier* Rb2, const FVector& FramePosition2,
		const FQuat& FrameRotation2) override;

private:
	FWheelJointBarrier(const FWheelJointBarrier&) = delete;
	void operator=(const FWheelJointBarrier&) = delete;
};
