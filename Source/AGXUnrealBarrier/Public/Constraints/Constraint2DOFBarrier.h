// Copyright 2023, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_Constraint2DOFFreeDOF.h"
#include "Constraints/ConstraintBarrier.h"

class FElectricMotorControllerBarrier;
class FFrictionControllerBarrier;
class FLockControllerBarrier;
class FRangeControllerBarrier;
class FScrewControllerBarrier;
class FTargetSpeedControllerBarrier;

namespace agx
{
	class Constraint2DOF;
}

class AGXUNREALBARRIER_API FConstraint2DOFBarrier : public FConstraintBarrier
{
public:
	FConstraint2DOFBarrier();
	FConstraint2DOFBarrier(FConstraint2DOFBarrier&& Other) = default;
	FConstraint2DOFBarrier(std::unique_ptr<FConstraintRef> Native);
	virtual ~FConstraint2DOFBarrier();

	double GetAngle(EAGX_Constraint2DOFFreeDOF Dof) const;

	TUniquePtr<FElectricMotorControllerBarrier> GetElectricMotorController(
		EAGX_Constraint2DOFFreeDOF Dof);
	TUniquePtr<FFrictionControllerBarrier> GetFrictionController(EAGX_Constraint2DOFFreeDOF Dof);
	TUniquePtr<FLockControllerBarrier> GetLockController(EAGX_Constraint2DOFFreeDOF Dof);
	TUniquePtr<FRangeControllerBarrier> GetRangeController(EAGX_Constraint2DOFFreeDOF Dof);
	TUniquePtr<FTargetSpeedControllerBarrier> GetTargetSpeedController(
		EAGX_Constraint2DOFFreeDOF Dof);
	TUniquePtr<FScrewControllerBarrier> GetScrewController();

	TUniquePtr<const FElectricMotorControllerBarrier> GetElectricMotorController(
		EAGX_Constraint2DOFFreeDOF Dof) const;
	TUniquePtr<const FFrictionControllerBarrier> GetFrictionController(
		EAGX_Constraint2DOFFreeDOF Dof) const;
	TUniquePtr<const FLockControllerBarrier> GetLockController(
		EAGX_Constraint2DOFFreeDOF Dof) const;
	TUniquePtr<const FRangeControllerBarrier> GetRangeController(
		EAGX_Constraint2DOFFreeDOF Dof) const;
	TUniquePtr<const FTargetSpeedControllerBarrier> GetTargetSpeedController(
		EAGX_Constraint2DOFFreeDOF Dof) const;
	TUniquePtr<const FScrewControllerBarrier> GetScrewController() const;

private:
	FConstraint2DOFBarrier(const FConstraint2DOFBarrier&) = delete;
	void operator=(const FConstraint2DOFBarrier&) = delete;
};
