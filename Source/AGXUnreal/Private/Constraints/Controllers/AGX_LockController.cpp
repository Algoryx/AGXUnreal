// Copyright 2025, Algoryx Simulation AB.

#include "Constraints/Controllers/AGX_LockController.h"

#include "Constraints/ControllerConstraintBarriers.h"


void FAGX_ConstraintLockController::InitializeBarrier(TUniquePtr<FLockControllerBarrier> Barrier)
{
	check(!HasNative());
	NativeBarrier = std::move(Barrier);
}

namespace
{
	FLockControllerBarrier* GetLockBarrier(FAGX_ConstraintLockController& Controller)
	{
		// See comment in GetElectricMotorController.
		return static_cast<FLockControllerBarrier*>(Controller.GetNative());
	}

	const FLockControllerBarrier* GetLockBarrier(const FAGX_ConstraintLockController& Controller)
	{
		// See comment in GetElectricMotorController.
		return static_cast<const FLockControllerBarrier*>(Controller.GetNative());
	}
}

void FAGX_ConstraintLockController::SetPosition(double InPosisiton)
{
	if (HasNative())
	{
		GetLockBarrier(*this)->SetPosition(InPosisiton);
	}
	Position = InPosisiton;
}

double FAGX_ConstraintLockController::GetPosition() const
{
	if (HasNative())
		return GetLockBarrier(*this)->GetPosition();

	return Position;
}

void FAGX_ConstraintLockController::UpdateNativePropertiesImpl()
{
	FLockControllerBarrier* Barrier = GetLockBarrier(*this);
	check(Barrier);
	Barrier->SetPosition(Position);
}

void FAGX_ConstraintLockController::CopyFrom(const FLockControllerBarrier& Source)
{
	Super::CopyFrom(Source);
	const double PositionBarrier = Source.GetPosition();
	Position = PositionBarrier;
}
