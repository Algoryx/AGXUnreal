// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "Constraints/ConstraintBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_ConstraintEnumsCommon.h"

// Unreal Engine includes.
#include "Templates/UniquePtr.h"

class FLockControllerBarrier;
class FRangeControllerBarrier;
class FTargetSpeedControllerBarrier;

struct FRigidBodyBarrier;

class AGXUNREALBARRIER_API FWheelJointBarrier : public FConstraintBarrier
{
public:
	FWheelJointBarrier();
	FWheelJointBarrier(FWheelJointBarrier&& Other) = default;
	FWheelJointBarrier(std::unique_ptr<FConstraintRef> Native);
	virtual ~FWheelJointBarrier();

	double GetAngle() const;

	TUniquePtr<FLockControllerBarrier> GetLockController(EAGX_WheelJointSecondaryConstraint Sc);
	TUniquePtr<FRangeControllerBarrier> GetRangeController(EAGX_WheelJointSecondaryConstraint Sc);
	TUniquePtr<FTargetSpeedControllerBarrier> GetTargetSpeedController(
		EAGX_WheelJointSecondaryConstraint Sc);

	TUniquePtr<const FLockControllerBarrier> GetLockController(
		EAGX_WheelJointSecondaryConstraint Sc) const;
	TUniquePtr<const FRangeControllerBarrier> GetRangeController(
		EAGX_WheelJointSecondaryConstraint Sc) const;
	TUniquePtr<const FTargetSpeedControllerBarrier> GetTargetSpeedController(
		EAGX_WheelJointSecondaryConstraint Sc) const;


private:
	virtual void AllocateNativeImpl(
		const FRigidBodyBarrier& Rb1, const FVector& FramePosition1, const FQuat& FrameRotation1,
		const FRigidBodyBarrier* Rb2, const FVector& FramePosition2,
		const FQuat& FrameRotation2) override;

private:
	FWheelJointBarrier(const FWheelJointBarrier&) = delete;
	void operator=(const FWheelJointBarrier&) = delete;
};
