// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Constraints/ElementaryConstraintBarrier.h"

// Unreal Engine includes.
#include "Math/Interval.h"

// Standard library includes.
#include <memory>

class AGXUNREALBARRIER_API FTwistRangeControllerBarrier : public FElementaryConstraintBarrier
{
public: // Type declarations.
	using Super = FElementaryConstraintBarrier;

public: // Special member functions.
	/// Create a Barrier without a Native but with a Native Ref.
	FTwistRangeControllerBarrier();

	/// Create a Barrier with the same Native as Other, but with a separate Native Ref.
	FTwistRangeControllerBarrier(const FTwistRangeControllerBarrier& Other);

	/// Downcast from base class. The passed instance must wrap a Twist Range Controller.
	FTwistRangeControllerBarrier(const FElementaryConstraintBarrier& Other);

	/// Create a Barrier taking ownership of the given Native Ref.
	/// The Native Ref, if non-nullptr, MUST point to a Twist Range Controller.
	FTwistRangeControllerBarrier(std::unique_ptr<FElementaryConstraintRef> InNative);

	virtual ~FTwistRangeControllerBarrier();

	/// Make this Barrier have the same Native as Other, but keep the Native Refs separate.
	FTwistRangeControllerBarrier& operator=(const FTwistRangeControllerBarrier& Other);

public: // Native management.
	virtual void SetNative(FElementaryConstraintRef* InNative) override;

public: // AGX Dynamics accessors.
	void SetRange(FDoubleInterval InRange);
	void SetRange(FAGX_RealInterval InRange);
	void SetRange(double InMin, double InMax);
	void SetRangeMin(double InMin);
	void SetRangeMax(double InMax);

	FDoubleInterval GetRange() const;
	double GetRangeMin() const;
	double GetRangeMax() const;

private:
	bool CheckValidNative();
};
