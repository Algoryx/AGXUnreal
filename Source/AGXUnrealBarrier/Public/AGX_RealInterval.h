// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Containers/UnrealString.h"
#include "Math/UnrealMathUtility.h"
#include "Math/Interval.h"

#include "AGX_RealInterval.generated.h"

/**
 * AGX_RealInterval mimics the Unreal Engine build-in FFloatInterval type, but
 * uses FAGX_Real instead of float for the underlying type. This extends the
 * floating point precision to 64-bit and provides support for scientific
 * notation and infinity in Details panels.
 */
USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FAGX_RealInterval
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Dynamics")
	FAGX_Real Min {0.0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Dynamics")
	FAGX_Real Max {0.0};

	FAGX_RealInterval() = default;

	FAGX_RealInterval(double InMin, double InMax)
		: Min(InMin)
		, Max(InMax)
	{
		Sort();
	}

	explicit FAGX_RealInterval(const double (&MinAndMax)[2])
		: FAGX_RealInterval(MinAndMax[0], MinAndMax[1])
	{
	}

	// Not explicit because we do want transparent transition between FAGX_RealInterval and
	// FDoubleInterval.
	FAGX_RealInterval(const FDoubleInterval& InInterval)
		: FAGX_RealInterval(InInterval.Min, InInterval.Max)
	{
	}

	explicit FAGX_RealInterval(double MinAndMax)
		: FAGX_RealInterval(-MinAndMax, MinAndMax)
	{
	}

	void Set(double InMin, double InMax)
	{
		Min = InMin;
		Max = InMax;
		Sort();
	}

	void Set(FDoubleInterval InInterval)
	{
		Min = InInterval.Min;
		Max = InInterval.Max;
	}

	void SetMin(double InMin)
	{
		Min = InMin;
	}

	void SetMax(double InMax)
	{
		Max = InMax;
	}

	void Get(double& OutMin, double& OutMax) const
	{
		OutMin = Min;
		OutMax = Max;
	}

	void Get(FDoubleInterval& OutInterval) const
	{
		OutInterval.Min = Min;
		OutInterval.Max = Max;
	}

	double GetMin() const
	{
		return Min;
	}

	double GetMax() const
	{
		return Max;
	}

	FDoubleInterval ToDouble() const
	{
		return FDoubleInterval(Min, Max);
	}

	operator FDoubleInterval() const
	{
		return ToDouble();
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

	FString ToString() const
	{
		return FString::Printf(TEXT("(Min=%g Max=%g"), Min.GetValue(), Max.GetValue());
	}

	bool Equals(const FAGX_RealInterval& Other, double Tolerance = UE_KINDA_SMALL_NUMBER) const
	{
		// Handle infinity, which the regular path fail on since infinity - infinity is NaN.
		if (Min == Other.Min && Max == Other.Max)
		{
			return true;
		}
		return FMath::Abs(Min - Other.Min) <= Tolerance && FMath::Abs(Max - Other.Max) <= Tolerance;
	}


	/// Called by Unreal Engine when de-serializing an FAGX_Real but some other type was found in
	/// the archive. FFloatInterval and FAGX_DoubleInterval are read but all other types are
	/// rejected.
	bool SerializeFromMismatchedTag(struct FPropertyTag const& Tag, FStructuredArchive::FSlot Slot);
};

inline bool operator==(const FAGX_RealInterval& Lhs, const FAGX_RealInterval& Rhs)
{
	return Lhs.Min == Rhs.Min && Lhs.Max == Rhs.Max;
}

/**
 * A struct that informs Unreal Engine about how FAGX_RealInterval can be used.
 */
template <>
struct TStructOpsTypeTraits<FAGX_RealInterval> : public TStructOpsTypeTraitsBase2<FAGX_RealInterval>
{
	// clang-format off
	enum
	{
		// This is the subset of flags from TSTructOpsTypeTraits that we care about for FAGX_RealInterval.
		// All other flags default to false.
		WithZeroConstructor            = true,           // struct can be constructed as a valid object by filling its memory footprint with zeroes.
		WithNoDestructor               = true,           // struct will not have its destructor called when it is destroyed.
		WithIdenticalViaEquality       = true,           // struct can be compared via its operator==.  This should be mutually exclusive with WithIdentical.
		// Tell Unreal Engine that while restoring an FAGX_Real it is OK to find a double or a float
		// since we can convert from double to FAGX_Real. This part doesn't specify the double bit,
		// just that the struct should at least get the chance to restore itself even when the type
		// in the archive isn't an FAGX_Real.
		WithStructuredSerializeFromMismatchedTag = true, // struct has an FStructuredArchive-based SerializeFromMismatchedTag function for converting from other property tags.
	};
	// clang-format on
};
