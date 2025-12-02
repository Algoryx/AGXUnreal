// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AMOR/MergeSplitThresholdsBarrier.h"

class FConstraintBarrier;

class AGXUNREALBARRIER_API FConstraintMergeSplitThresholdsBarrier
	: public FMergeSplitThresholdsBarrier
{
public:
	FConstraintMergeSplitThresholdsBarrier();
	FConstraintMergeSplitThresholdsBarrier(FConstraintMergeSplitThresholdsBarrier&& Other) =
		default;
	FConstraintMergeSplitThresholdsBarrier(std::unique_ptr<FMergeSplitThresholdsRef>&& Native);
	~FConstraintMergeSplitThresholdsBarrier();

	void AllocateNative(bool bInIsRotational);

	void SetMaxDesiredForceRangeDiff(double InMaxDesiredForceRangeDiff);
	double GetMaxDesiredForceRangeDiff() const;

	void SetMaxDesiredLockAngleDiff(double InMaxDesiredLockAngleDiff);
	double GetMaxDesiredLockAngleDiff() const;

	void SetMaxDesiredRangeAngleDiff(double InMaxDesiredRangeAngleDiff);
	double GetMaxDesiredRangeAngleDiff() const;

	void SetMaxDesiredSpeedDiff(double InMaxDesiredSpeedDiff);
	double GetMaxDesiredSpeedDiff() const;

	void SetMaxRelativeSpeed(double InMaxRelativeSpeed);
	double GetMaxRelativeSpeed() const;

	/**
	 * Control whether unit conversion is done between radians and degrees or  between meters and
	 * centimeters.
	 */
	void SetIsRotational(bool InIsRotational);

	/**
	 * Returns true if unit conversions is done between radians and degrees, false if meters and
	 * centimeters.
	 */
	bool GetIsRotational() const;

	static FConstraintMergeSplitThresholdsBarrier CreateFrom(const FConstraintBarrier& Barrier);

private:
	FConstraintMergeSplitThresholdsBarrier(const FConstraintMergeSplitThresholdsBarrier&) = delete;
	void operator=(const FConstraintMergeSplitThresholdsBarrier&) = delete;

	/**
	 * Control whether this Merge Split Thresholds act on a rotational or translational degree of
	 * freedom. Not part of the AGX Dynamics state, used to determine if unit conversions should be
	 * done on an angle or a distance.
	 */
	bool bIsRotational = true;
};
