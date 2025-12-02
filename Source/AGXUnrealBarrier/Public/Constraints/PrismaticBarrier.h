// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "Constraints/Constraint1DOFBarrier.h"

struct FRigidBodyBarrier;

class AGXUNREALBARRIER_API FPrismaticBarrier : public FConstraint1DOFBarrier
{
public:
	FPrismaticBarrier();
	FPrismaticBarrier(FPrismaticBarrier&& Other) = default;
	FPrismaticBarrier(std::unique_ptr<FConstraintRef> Native);
	virtual ~FPrismaticBarrier();

private:
	virtual void AllocateNativeImpl(
		const FRigidBodyBarrier& Rb1, const FVector& FramePosition1, const FQuat& FrameRotation1,
		const FRigidBodyBarrier* Rb2, const FVector& FramePosition2,
		const FQuat& FrameRotation2) override;

private:
	FPrismaticBarrier(const FPrismaticBarrier&) = delete;
	void operator=(const FPrismaticBarrier&) = delete;
};
