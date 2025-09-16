// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "Constraints/ConstraintBarrier.h"

struct FRigidBodyBarrier;

struct FConstraintRef;

/**
 * A Constraint Barrier that may wrap any AGX Dynamics Constraint type.
 *
 * Cannot be used to allocate new AGX Dynamics Constraints, since it doesn't know what type of
 * Constraint to allocate.
 */
class AGXUNREALBARRIER_API FAnyConstraintBarrier : public FConstraintBarrier
{
public:
	FAnyConstraintBarrier();
	FAnyConstraintBarrier(std::unique_ptr<FConstraintRef> Native);
	FAnyConstraintBarrier(FAnyConstraintBarrier&& Other);
	virtual ~FAnyConstraintBarrier() override;

private:
	/**
	 * Will always fail with an error message.
	 */
	virtual void AllocateNativeImpl(
		const FRigidBodyBarrier& RigidBody1, const FVector& FramePosition1,
		const FQuat& FrameRotation1, const FRigidBodyBarrier* RigidBody2,
		const FVector& FramePosition2, const FQuat& FrameRotation2) override;

private:
	FAnyConstraintBarrier(const FAnyConstraintBarrier&) = delete;
	void operator=(const FAnyConstraintBarrier&) = delete;
};
