// Copyright 2024, Algoryx Simulation AB.

#include "Constraints/Controllers/AGX_TwistRangeController.h"

// AGX Dynamics for Unreal includes.

// Special member functions.

FAGX_TwistRangeController::FAGX_TwistRangeController()
{
	bEnable = false;
}

FAGX_TwistRangeController::~FAGX_TwistRangeController()
{
}

// Properties.

void FAGX_TwistRangeController::SetRange(FDoubleInterval InRange)
{
	if (HasNative())
	{
		Barrier.SetRange(InRange);
	}
	Range = FAGX_RealInterval {InRange.Min, InRange.Max};
}

void FAGX_TwistRangeController::SetRange(FAGX_RealInterval InRange)
{
	SetRange(InRange.ToDouble());
}

void FAGX_TwistRangeController::SetRange(double InRangeMin, double InRangeMax)
{
	SetRange(FDoubleInterval{InRangeMin, InRangeMax});
}

void FAGX_TwistRangeController::SetRangeMin(double InMin)
{
	if (HasNative())
	{
		Barrier.SetRangeMin(InMin);
	}
	Range.Min = InMin;
}

void FAGX_TwistRangeController::SetRangeMax(double InMax)
{
	if (HasNative())
	{
		Barrier.SetRangeMax(InMax);
	}
	Range.Max = InMax;
}

FDoubleInterval FAGX_TwistRangeController::GetRange() const
{
	if (HasNative())
	{
		return Barrier.GetRange();
	}
	return Range;
}

double FAGX_TwistRangeController::GetRangeMin() const
{
	if (HasNative())
	{
		return Barrier.GetRangeMin();
	}
	return Range.Min;
}

double FAGX_TwistRangeController::GetRangeMax() const
{
	if (HasNative())
	{
		return Barrier.GetRangeMax();
	}
	return Range.Max;
}

// Native management.

bool FAGX_TwistRangeController::HasNative() const
{
	check(Super::HasNative() == Barrier.HasNative());
	return Barrier.HasNative();
}

FTwistRangeControllerBarrier* FAGX_TwistRangeController::GetNative()
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &Barrier;
}

const FTwistRangeControllerBarrier* FAGX_TwistRangeController::GetNative() const
{
	if (!HasNative())
	{
		return nullptr;
	}
	return &Barrier;
}

void FAGX_TwistRangeController::InitializeBarrier(const FTwistRangeControllerBarrier& InBarrier)
{
	check(!HasNative());
	Super::InitializeBarrier(InBarrier);
	Barrier = InBarrier;
}

void FAGX_TwistRangeController::CopyFrom(const FTwistRangeControllerBarrier& Source)
{
	Super::CopyFrom(Source);

	const FAGX_RealInterval RangeBarrier = Source.GetRange();
	Range = RangeBarrier;
}

// Member function overrides.

void FAGX_TwistRangeController::UpdateNativeProperties()
{
	check(HasNative());
	Super::UpdateNativeProperties();
	Barrier.SetRange(Range);
}

void FAGX_TwistRangeController::InitializeBarrier(const FElementaryConstraintBarrier& InBarrier)
{
	InitializeBarrier(FTwistRangeControllerBarrier(InBarrier));
}
