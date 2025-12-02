// Copyright 2025, Algoryx Simulation AB.

#include "Constraints/Controllers/AGX_RangeController.h"

#include "Constraints/AGX_ConstraintConstants.h"
#include "Constraints/ControllerConstraintBarriers.h"

FAGX_ConstraintRangeController::FAGX_ConstraintRangeController()
	: Range(ConstraintConstants::DefaultForceRange())
{
}

void FAGX_ConstraintRangeController::InitializeBarrier(TUniquePtr<FRangeControllerBarrier> Barrier)
{
	check(!HasNative());
	NativeBarrier = std::move(Barrier);
}

namespace
{
	FRangeControllerBarrier* GetRangeBarrier(FAGX_ConstraintRangeController& Controller)
	{
		// See comment in GetElectricMotorBarrier.
		return static_cast<FRangeControllerBarrier*>(Controller.GetNative());
	}

	const FRangeControllerBarrier* GetRangeBarrier(const FAGX_ConstraintRangeController& Controller)
	{
		// See comment in GetElectricMotorBarrier.
		return static_cast<const FRangeControllerBarrier*>(Controller.GetNative());
	}
}

void FAGX_ConstraintRangeController::SetRange(const FAGX_RealInterval& InRange)
{
	if (HasNative())
	{
		GetRangeBarrier(*this)->SetRange(InRange);
	}
	Range = InRange;
}

void FAGX_ConstraintRangeController::SetRange(double RangeMin, double RangeMax)
{
	SetRange(FAGX_RealInterval(RangeMin, RangeMax));
}

FAGX_RealInterval FAGX_ConstraintRangeController::GetRange() const
{
	if (HasNative())
	{
		return GetRangeBarrier(*this)->GetRange();
	}
	return Range;
}

void FAGX_ConstraintRangeController::UpdateNativePropertiesImpl()
{
	FRangeControllerBarrier* Barrier = GetRangeBarrier(*this);
	check(Barrier);
	Barrier->SetRange(Range);
}

void FAGX_ConstraintRangeController::CopyFrom(const FRangeControllerBarrier& Source)
{
	Super::CopyFrom(Source);
	const FAGX_RealInterval RangeBarrier = Source.GetRange();
	Range = RangeBarrier;
}
