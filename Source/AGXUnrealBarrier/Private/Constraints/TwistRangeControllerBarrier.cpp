// Copyright 2025, Algoryx Simulation AB.

#include "Constraints/TwistRangeControllerBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "BarrierOnly/AGXRefs.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/Constraint.h>
#include "EndAGXIncludes.h"

// Special member functions.

FTwistRangeControllerBarrier::FTwistRangeControllerBarrier()
{
}

FTwistRangeControllerBarrier::FTwistRangeControllerBarrier(
	const FTwistRangeControllerBarrier& Other)
	: FElementaryConstraintBarrier(
		  std::make_unique<FElementaryConstraintRef>(Other.NativeRef->Native))
{
}

FTwistRangeControllerBarrier::FTwistRangeControllerBarrier(
	const FElementaryConstraintBarrier& Other)
	: FElementaryConstraintBarrier(Other)
{
	if (!CheckValidNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("A Twist Range Controller Barrier was created from an Elementarcy Constraint "
				 "Barrier that has a Native that is not a Twist Range Controller"));
	}
}

FTwistRangeControllerBarrier::FTwistRangeControllerBarrier(
	std::unique_ptr<FElementaryConstraintRef> InNative)
	: FElementaryConstraintBarrier(std::move(InNative))
{
	if (!CheckValidNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("A Twist Range Controller Barrier was created from an Elementary Constraint of "
				 "incompatible type. This is not allowed."));
	}
}

FTwistRangeControllerBarrier::~FTwistRangeControllerBarrier()
{
	// Must have a non-inlined destructor because the NativeRef destructor must be able to see the
	// full definition of the pointed-to type, and we are not allowed to include F*Ref / F*Ptr
	// types in the header file.
}

FTwistRangeControllerBarrier& FTwistRangeControllerBarrier::operator=(
	const FTwistRangeControllerBarrier& Other)
{
	Super::operator=(Other);
	return *this;
}

void FTwistRangeControllerBarrier::SetNative(FElementaryConstraintRef* InNative)
{
	Super::SetNative(InNative);
	CheckValidNative();
}

namespace
{
	agx::TwistRangeController* GetNative(FTwistRangeControllerBarrier& Barrier)
	{
		return static_cast<agx::TwistRangeController*>(Barrier.GetNative()->Native.get());
	}

	const agx::TwistRangeController* GetNative(const FTwistRangeControllerBarrier& Barrier)
	{
		return static_cast<const agx::TwistRangeController*>(Barrier.GetNative()->Native.get());
	}
}

// AGX Dynamics accessors.

void FTwistRangeControllerBarrier::SetRange(FDoubleInterval InRange)
{
	check(HasNative());
	const agx::RangeReal RangeAGX = ConvertAngle(InRange);
	::GetNative(*this)->setRange(RangeAGX);
}

void FTwistRangeControllerBarrier::SetRange(FAGX_RealInterval InRange)
{
	SetRange(InRange.ToDouble());
}

void FTwistRangeControllerBarrier::SetRange(double InMin, double InMax)
{
	SetRange(FDoubleInterval {InMin, InMax});
}

void FTwistRangeControllerBarrier::SetRangeMin(double InMin)
{
	check(HasNative());
	const agx::Real MinAGX = ConvertAngleToAGX(InMin);
	::GetNative(*this)->getRange().lower() = MinAGX;
}

void FTwistRangeControllerBarrier::SetRangeMax(double InMax)
{
	check(HasNative());
	const agx::Real MaxAGX = ConvertAngleToAGX(InMax);
	::GetNative(*this)->getRange().upper() = MaxAGX;
}

FDoubleInterval FTwistRangeControllerBarrier::GetRange() const
{
	check(HasNative());
	const agx::RangeReal RangeAGX = ::GetNative(*this)->getRange();
	const FDoubleInterval Range = ConvertAngle(RangeAGX);
	return Range;
}

double FTwistRangeControllerBarrier::GetRangeMin() const
{
	check(HasNative());
	const agx::Real MinAGX = ::GetNative(*this)->getRange().lower();
	const double Min = ConvertAngleToUnreal<double>(MinAGX);
	return Min;
}

double FTwistRangeControllerBarrier::GetRangeMax() const
{
	check(HasNative());
	const agx::Real MaxAGX = ::GetNative(*this)->getRange().upper();
	const double Max = ConvertAngleToUnreal<double>(MaxAGX);
	return Max;
}

bool FTwistRangeControllerBarrier::CheckValidNative()
{
	check(!HasNative() || NativeRef->Native->is<agx::TwistRangeController>());
	if (HasNative() && !NativeRef->Native->is<agx::TwistRangeController>())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Invalid Elementary Constraint detected in Twist Range Controller Barrier. Native "
				 "cleared."));
		NativeRef->Native = nullptr;
		return false;
	}
	return true;
}
