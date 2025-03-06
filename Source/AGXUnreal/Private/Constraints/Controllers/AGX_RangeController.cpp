// Copyright 2024, Algoryx Simulation AB.

#include "Constraints/Controllers/AGX_RangeController.h"

#include "Constraints/AGX_ConstraintConstants.h"
#include "Constraints/ControllerConstraintBarriers.h"

FAGX_ConstraintRangeController::FAGX_ConstraintRangeController(bool bRotational)
	: FAGX_ConstraintController(bRotational)
	, Range(ConstraintConstants::DefaultForceRange())
{
}

void FAGX_ConstraintRangeController::InitializeBarrier(TUniquePtr<FRangeControllerBarrier> Barrier)
{
	check(!HasNative());
	NativeBarrier = std::move(Barrier);
	check(HasNative());
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
		if (bRotational)
		{
			GetRangeBarrier(*this)->SetRangeRotational(InRange);
		}
		else
		{
			GetRangeBarrier(*this)->SetRangeTranslational(InRange);
		}
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
		if (bRotational)
		{
			return GetRangeBarrier(*this)->GetRangeRotational();
		}
		else
		{
			return GetRangeBarrier(*this)->GetRangeTranslational();
		}
	}
	return Range;
}

void FAGX_ConstraintRangeController::UpdateNativePropertiesImpl()
{
	FRangeControllerBarrier* Barrier = GetRangeBarrier(*this);
	check(Barrier);
	if (bRotational)
	{
		Barrier->SetRangeRotational(Range);
	}
	else
	{
		Barrier->SetRangeTranslational(Range);
	}
}

void FAGX_ConstraintRangeController::CopyFrom(const FRangeControllerBarrier& Source)
{
	Super::CopyFrom(Source);

	const FAGX_RealInterval RangeBarrier =
		bRotational ? Source.GetRangeRotational() : Source.GetRangeTranslational();

	Range = RangeBarrier;
}
