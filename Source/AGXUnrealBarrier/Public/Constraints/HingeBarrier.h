// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "Constraints/Constraint1DOFBarrier.h"

struct FRigidBodyBarrier;

class AGXUNREALBARRIER_API FHingeBarrier : public FConstraint1DOFBarrier
{
public:
	FHingeBarrier();
	FHingeBarrier(FHingeBarrier&& Other) = default;
	FHingeBarrier(std::unique_ptr<FConstraintRef> Native);
	virtual ~FHingeBarrier();

private:
	virtual void AllocateNativeImpl(
		const FRigidBodyBarrier& Rb1, const FVector& FramePosition1, const FQuat& FrameRotation1,
		const FRigidBodyBarrier* Rb2, const FVector& FramePosition2,
		const FQuat& FrameRotation2) override;

private:
	FHingeBarrier(const FHingeBarrier&) = delete;
	void operator=(const FHingeBarrier&) = delete;
};
