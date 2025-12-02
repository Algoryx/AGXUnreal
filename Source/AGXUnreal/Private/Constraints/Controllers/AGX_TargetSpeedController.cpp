// Copyright 2025, Algoryx Simulation AB.

#include "Constraints/Controllers/AGX_TargetSpeedController.h"

#include "Constraints/AGX_ConstraintConstants.h"
#include "Constraints/ControllerConstraintBarriers.h"


void FAGX_ConstraintTargetSpeedController::InitializeBarrier(
	TUniquePtr<FTargetSpeedControllerBarrier> Barrier)
{
	check(!HasNative());
	NativeBarrier = std::move(Barrier);
}

namespace
{
	FTargetSpeedControllerBarrier* GetSpeedBarrier(FAGX_ConstraintTargetSpeedController& Controller)
	{
		// See comment in GetElectricMotorController.
		return static_cast<FTargetSpeedControllerBarrier*>(Controller.GetNative());
	}

	const FTargetSpeedControllerBarrier* GetSpeedBarrier(
		const FAGX_ConstraintTargetSpeedController& Controller)
	{
		// See comment in GetElectricMotorController.
		return static_cast<const FTargetSpeedControllerBarrier*>(Controller.GetNative());
	}
}

void FAGX_ConstraintTargetSpeedController::SetSpeed(double InSpeed)
{
	if (HasNative())
	{
		GetSpeedBarrier(*this)->SetSpeed(InSpeed);
	}
	Speed = InSpeed;
}

double FAGX_ConstraintTargetSpeedController::GetSpeed() const
{
	if (HasNative())
	{
		return GetSpeedBarrier(*this)->GetSpeed();
	}
	return Speed;
}

void FAGX_ConstraintTargetSpeedController::SetLockedAtZeroSpeed(bool bInLockedAtZeroSpeed)
{
	if (HasNative())
	{
		GetSpeedBarrier(*this)->SetLockedAtZeroSpeed(bInLockedAtZeroSpeed);
	}
	bLockedAtZeroSpeed = bInLockedAtZeroSpeed;
}

bool FAGX_ConstraintTargetSpeedController::GetLockedAtZeroSpeed() const
{
	if (HasNative())
	{
		return GetSpeedBarrier(*this)->GetLockedAtZeroSpeed();
	}
	else
	{
		return bLockedAtZeroSpeed;
	}
}

void FAGX_ConstraintTargetSpeedController::UpdateNativePropertiesImpl()
{
	FTargetSpeedControllerBarrier* Barrier = GetSpeedBarrier(*this);
	check(Barrier);
	Barrier->SetLockedAtZeroSpeed(bLockedAtZeroSpeed);
	Barrier->SetSpeed(Speed);
}

void FAGX_ConstraintTargetSpeedController::CopyFrom(
	const FTargetSpeedControllerBarrier& Source)
{
	Super::CopyFrom(Source);
	bLockedAtZeroSpeed = Source.GetLockedAtZeroSpeed();
	Speed = Source.GetSpeed();
}
