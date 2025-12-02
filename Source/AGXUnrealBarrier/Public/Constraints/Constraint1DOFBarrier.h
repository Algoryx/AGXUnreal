// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Constraints/ConstraintBarrier.h"

// Unreal Engine includes.
#include "Templates/UniquePtr.h"

class FElectricMotorControllerBarrier;
class FFrictionControllerBarrier;
class FLockControllerBarrier;
class FRangeControllerBarrier;
class FTargetSpeedControllerBarrier;

namespace agx
{
	class Constraint1DOF;
}

class AGXUNREALBARRIER_API FConstraint1DOFBarrier : public FConstraintBarrier
{
public:
	FConstraint1DOFBarrier();
	FConstraint1DOFBarrier(FConstraint1DOFBarrier&& Other) = default;
	FConstraint1DOFBarrier(std::unique_ptr<FConstraintRef> Native);
	virtual ~FConstraint1DOFBarrier();

	/**
	 * Get the current angle of the free degree of freedom is this constraint. For a Hinge this
	 * is an angle measured in degrees and for a prismatic this is a distance measured in
	 * centimeters.
	 * @return The current angle of the free degree of freedom.
	 */
	double GetAngle() const;

	/**
	 * This methods return the current speed for the 1D constraint.
	 *
	 * For a prismatic this is the linear velocity along the axis, if it is a hinge, then it is the
	 * current angular rotation around the hinge axis.
	 */
	double GetSpeed() const;

	TUniquePtr<FElectricMotorControllerBarrier> GetElectricMotorController();
	TUniquePtr<FFrictionControllerBarrier> GetFrictionController();
	TUniquePtr<FLockControllerBarrier> GetLockController();
	TUniquePtr<FRangeControllerBarrier> GetRangeController();
	TUniquePtr<FTargetSpeedControllerBarrier> GetTargetSpeedController();

	TUniquePtr<const FElectricMotorControllerBarrier> GetElectricMotorController() const;
	TUniquePtr<const FFrictionControllerBarrier> GetFrictionController() const;
	TUniquePtr<const FLockControllerBarrier> GetLockController() const;
	TUniquePtr<const FRangeControllerBarrier> GetRangeController() const;
	TUniquePtr<const FTargetSpeedControllerBarrier> GetTargetSpeedController() const;

private:
	FConstraint1DOFBarrier(const FConstraint1DOFBarrier&) = delete;
	void operator=(const FConstraint1DOFBarrier&) = delete;
};
