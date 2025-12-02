// Copyright 2025, Algoryx Simulation AB.

#include "AMOR/ConstraintMergeSplitThresholdsBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "BarrierOnly/AGXRefs.h"
#include "Constraints/ConstraintBarrier.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSDK/MergeSplitHandler.h>
#include "EndAGXIncludes.h"

FConstraintMergeSplitThresholdsBarrier::FConstraintMergeSplitThresholdsBarrier()
	: FMergeSplitThresholdsBarrier()
{
}

FConstraintMergeSplitThresholdsBarrier::FConstraintMergeSplitThresholdsBarrier(
	std::unique_ptr<FMergeSplitThresholdsRef>&& Native)
	: FMergeSplitThresholdsBarrier(std::move(Native))
{
}

FConstraintMergeSplitThresholdsBarrier::~FConstraintMergeSplitThresholdsBarrier()
{
}

void FConstraintMergeSplitThresholdsBarrier::AllocateNative(bool bInIsRotational)
{
	check(!HasNative());
	bIsRotational = bInIsRotational;
	NativeRef->Native = new agxSDK::ConstraintMergeSplitThresholds();
}

namespace ConstraintMergeSplitThresholds_helpers
{
	agxSDK::ConstraintMergeSplitThresholds* CastToConstraintThresholds(
		agxSDK::MergeSplitThresholds* Thresholds, const FString& Operation)
	{
		agxSDK::ConstraintMergeSplitThresholds* ConstraintThresholds =
			dynamic_cast<agxSDK::ConstraintMergeSplitThresholds*>(Thresholds);
		if (ConstraintThresholds == nullptr)
		{
			UE_LOG(
				LogAGX, Error,
				TEXT("Operation %s failed, could not cast native MergeSplitThresholds to "
					 "ConstraintMergeSplitThresholds."),
				*Operation);
		}

		return ConstraintThresholds;
	}
}

void FConstraintMergeSplitThresholdsBarrier::SetMaxDesiredForceRangeDiff(
	double InMaxDesiredForceRangeDiff)
{
	check(HasNative());
	using namespace ConstraintMergeSplitThresholds_helpers;
	if (auto Native = CastToConstraintThresholds(NativeRef->Native, "SetMaxDesiredForceRangeDiff"))
	{
		Native->setMaxDesiredForceRangeDiff(InMaxDesiredForceRangeDiff);
	}
}

double FConstraintMergeSplitThresholdsBarrier::GetMaxDesiredForceRangeDiff() const
{
	check(HasNative());
	using namespace ConstraintMergeSplitThresholds_helpers;
	if (auto Native = CastToConstraintThresholds(NativeRef->Native, "GetMaxDesiredForceRangeDiff"))
	{
		return Native->getMaxDesiredForceRangeDiff();
	}

	// Error message printed above.
	return 0.0;
}

void FConstraintMergeSplitThresholdsBarrier::SetMaxDesiredLockAngleDiff(
	double InMaxDesiredLockAngleDiff)
{
	check(HasNative());
	using namespace ConstraintMergeSplitThresholds_helpers;
	if (auto Native = CastToConstraintThresholds(NativeRef->Native, "SetMaxDesiredLockAngleDiff"))
	{
		const agx::Real ValAGX = bIsRotational ? ConvertAngleToAGX(InMaxDesiredLockAngleDiff)
											   : ConvertDistanceToAGX(InMaxDesiredLockAngleDiff);
		Native->setMaxDesiredLockAngleDiff(ValAGX);
	}
}

double FConstraintMergeSplitThresholdsBarrier::GetMaxDesiredLockAngleDiff() const
{
	check(HasNative());
	using namespace ConstraintMergeSplitThresholds_helpers;
	if (auto Native = CastToConstraintThresholds(NativeRef->Native, "GetMaxDesiredLockAngleDiff"))
	{
		const agx::Real ValAGX = Native->getMaxDesiredLockAngleDiff();
		return bIsRotational ? ConvertAngleToUnreal<double>(ValAGX)
							 : ConvertDistanceToUnreal<double>(ValAGX);
	}

	// Error message printed above.
	return 0.0;
}

void FConstraintMergeSplitThresholdsBarrier::SetMaxDesiredRangeAngleDiff(
	double InMaxDesiredRangeAngleDiff)
{
	check(HasNative());
	using namespace ConstraintMergeSplitThresholds_helpers;
	if (auto Native = CastToConstraintThresholds(NativeRef->Native, "SetMaxDesiredRangeAngleDiff"))
	{
		const agx::Real ValAGX = bIsRotational ? ConvertAngleToAGX(InMaxDesiredRangeAngleDiff)
											   : ConvertDistanceToAGX(InMaxDesiredRangeAngleDiff);
		Native->setMaxDesiredRangeAngleDiff(ValAGX);
	}
}

double FConstraintMergeSplitThresholdsBarrier::GetMaxDesiredRangeAngleDiff() const
{
	check(HasNative());
	using namespace ConstraintMergeSplitThresholds_helpers;
	if (auto Native = CastToConstraintThresholds(NativeRef->Native, "GetMaxDesiredRangeAngleDiff"))
	{
		const agx::Real ValAGX = Native->getMaxDesiredRangeAngleDiff();
		return bIsRotational ? ConvertAngleToUnreal<double>(ValAGX)
							 : ConvertDistanceToUnreal<double>(ValAGX);
	}

	// Error message printed above.
	return 0.0;
}

void FConstraintMergeSplitThresholdsBarrier::SetMaxDesiredSpeedDiff(double InMaxDesiredSpeedDiff)
{
	check(HasNative());
	using namespace ConstraintMergeSplitThresholds_helpers;
	if (auto Native = CastToConstraintThresholds(NativeRef->Native, "SetMaxDesiredSpeedDiff"))
	{
		const agx::Real ValAGX = bIsRotational ? ConvertAngleToAGX(InMaxDesiredSpeedDiff)
											   : ConvertDistanceToAGX(InMaxDesiredSpeedDiff);
		Native->setMaxDesiredSpeedDiff(ValAGX);
	}
}

double FConstraintMergeSplitThresholdsBarrier::GetMaxDesiredSpeedDiff() const
{
	check(HasNative());
	using namespace ConstraintMergeSplitThresholds_helpers;
	if (auto Native = CastToConstraintThresholds(NativeRef->Native, "GetMaxDesiredSpeedDiff"))
	{
		const agx::Real ValAGX = Native->getMaxDesiredSpeedDiff();
		return bIsRotational ? ConvertAngleToUnreal<double>(ValAGX)
							 : ConvertDistanceToUnreal<double>(ValAGX);
	}

	// Error message printed above.
	return 0.0;
}

void FConstraintMergeSplitThresholdsBarrier::SetMaxRelativeSpeed(double InMaxRelativeSpeed)
{
	check(HasNative());
	using namespace ConstraintMergeSplitThresholds_helpers;
	if (auto Native = CastToConstraintThresholds(NativeRef->Native, "SetMaxRelativeSpeed"))
	{
		const agx::Real ValAGX = bIsRotational ? ConvertAngleToAGX(InMaxRelativeSpeed)
											   : ConvertDistanceToAGX(InMaxRelativeSpeed);
		Native->setMaxRelativeSpeed(ValAGX);
	}
}

double FConstraintMergeSplitThresholdsBarrier::GetMaxRelativeSpeed() const
{
	check(HasNative());
	using namespace ConstraintMergeSplitThresholds_helpers;
	if (auto Native = CastToConstraintThresholds(NativeRef->Native, "GetMaxRelativeSpeed"))
	{
		const agx::Real ValAGX = Native->getMaxRelativeSpeed();
		return bIsRotational ? ConvertAngleToUnreal<double>(ValAGX)
							 : ConvertDistanceToUnreal<double>(ValAGX);
	}

	// Error message printed above.
	return 0.0;
}

void FConstraintMergeSplitThresholdsBarrier::SetIsRotational(bool InIsRotational)
{
	bIsRotational = InIsRotational;
}

bool FConstraintMergeSplitThresholdsBarrier::GetIsRotational() const
{
	return bIsRotational;
}

FConstraintMergeSplitThresholdsBarrier FConstraintMergeSplitThresholdsBarrier::CreateFrom(
	const FConstraintBarrier& Barrier)
{
	if (!Barrier.HasNative())
	{
		return FConstraintMergeSplitThresholdsBarrier();
	}

	const auto Msp = agxSDK::MergeSplitHandler::getProperties(Barrier.GetNative()->Native);
	if (Msp == nullptr)
	{
		return FConstraintMergeSplitThresholdsBarrier();
	}

	auto Mst = Msp->getConstraintThresholds();
	if (Mst == nullptr)
	{
		return FConstraintMergeSplitThresholdsBarrier();
	}

	FConstraintMergeSplitThresholdsBarrier ThresholdsBarrier(
		std::make_unique<FMergeSplitThresholdsRef>(Mst));
	ThresholdsBarrier.SetIsRotational(Barrier.IsRotational());
	return ThresholdsBarrier;
}
