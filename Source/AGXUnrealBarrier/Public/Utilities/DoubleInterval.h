// Copyright 2022, Algoryx Simulation AB.


#pragma once

// Unreal Engine includes.
#include "Math/Interval.h"
#include "Math/UnrealMathUtility.h"

// Standard library includes.
#include <algorithm>

#include "DoubleInterval.generated.h"

#if 0

/// @todo Replace this with the Unreal Engine double interval once they provide one.
/// This type is mostly useless since it can't be used as a UPROPERTY since it's not a USTRUCT.
/// They must be doing something custom with FFloatInterval.
/// We can perhaps do a double interval from scratch, with the USTRUCT decorator. Unsure how much
/// GUI-work that would entail.
DEFINE_INTERVAL_WRAPPER_STRUCT(FAGX_DoubleInterval, double)

#else

USTRUCT()
struct AGXUNREALBARRIER_API FAGX_DoubleInterval
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Interval")
	double Min = 0.0;

	UPROPERTY(EditAnywhere, Category = "Interval")
	double Max = 0.0;

	FAGX_DoubleInterval() = default;

	FAGX_DoubleInterval(double InMin, double InMax)
		: Min(InMin)
		, Max(InMax)
	{
		Sort();
	}

	explicit FAGX_DoubleInterval(double MinAndMax)
		: Min(-MinAndMax)
		, Max(MinAndMax)
	{
		Sort();
	}

	explicit FAGX_DoubleInterval(const double (&MinAndMax)[2])
		: Min(MinAndMax[0])
		, Max(MinAndMax[1])
	{
		Sort();
	}

	bool IsNearlyZero(double Tolerance = SMALL_NUMBER) const
	{
		return FMath::IsNearlyZero(Min, Tolerance) && FMath::IsNearlyZero(Max, Tolerance);
	}

	bool IsZero() const
	{
		return Min == 0.0 && Max == 0.0;
	}

	void Sort()
	{
		if (Min > Max)
		{
			std::swap(Min, Max);
		}
	}
};

#endif
