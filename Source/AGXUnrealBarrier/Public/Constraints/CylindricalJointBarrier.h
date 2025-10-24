// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "Constraints/Constraint2DOFBarrier.h"

struct FRigidBodyBarrier;

class AGXUNREALBARRIER_API FCylindricalJointBarrier : public FConstraint2DOFBarrier
{
public:
	FCylindricalJointBarrier();
	FCylindricalJointBarrier(FCylindricalJointBarrier&& Other) = default;
	FCylindricalJointBarrier(std::unique_ptr<FConstraintRef> Native);
	virtual ~FCylindricalJointBarrier();

private:
	virtual void AllocateNativeImpl(
		const FRigidBodyBarrier& Rb1, const FVector& FramePosition1, const FQuat& FrameRotation1,
		const FRigidBodyBarrier* Rb2, const FVector& FramePosition2,
		const FQuat& FrameRotation2) override;

private:
	FCylindricalJointBarrier(const FCylindricalJointBarrier&) = delete;
	void operator=(const FCylindricalJointBarrier&) = delete;
};
