// Copyright 2024, Algoryx Simulation AB.

#pragma once

#include "Constraints/AGX_ConstraintEnumsCommon.h"
#include "Constraints/Constraint1DOFBarrier.h"

class FRigidBodyBarrier;
class FConstraintControllerBarrier;

class AGXUNREALBARRIER_API FSingleControllerConstraint1DOFBarrier : public FConstraint1DOFBarrier
{
public:
	FSingleControllerConstraint1DOFBarrier();
	FSingleControllerConstraint1DOFBarrier(FSingleControllerConstraint1DOFBarrier&& Other) =
		default;
	FSingleControllerConstraint1DOFBarrier(std::unique_ptr<FConstraintRef> Native);
	virtual ~FSingleControllerConstraint1DOFBarrier();

	/// The SingleControllerConstraint1DOF needs a special AllocateNative method to correctly set up
	/// the Constraint Controller, since it only has one.
	void AllocateNative(
		const FRigidBodyBarrier& Rb1, const FVector& FramePosition1, const FQuat& FrameRotation1,
		const FRigidBodyBarrier* Rb2, const FVector& FramePosition2, const FQuat& FrameRotation2,
		FConstraintControllerBarrier* Controller,
		EAGX_ConstraintControllerType ControllerType,
		EAGX_ConstraintAngleControllerType ControllerAngleType);

	EAGX_ConstraintControllerType GetControllerType() const;
	EAGX_ConstraintAngleControllerType GetControllerAngleType() const;

private:
	virtual void AllocateNativeImpl(
		const FRigidBodyBarrier& Rb1, const FVector& FramePosition1, const FQuat& FrameRotation1,
		const FRigidBodyBarrier* Rb2, const FVector& FramePosition2,
		const FQuat& FrameRotation2) override;

private:
	FSingleControllerConstraint1DOFBarrier(const FSingleControllerConstraint1DOFBarrier&) = delete;
	void operator=(const FSingleControllerConstraint1DOFBarrier&) = delete;
};
