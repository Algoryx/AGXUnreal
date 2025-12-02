// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Class.h"

#include "AGX_Real.generated.h"

/**
 * A simple wrapper type that holds a double, the native floating-point type in AGX Dynamics.
 *
 * Has been designed to be transparently used as a double in AGX Dynamics for Unreal code. It is
 * meant to be use a UObject subclass properties only, not as parameters, return values, or
 * temporary variables.
 *
 * Required because Unreal Editor doesn't handle double very well. There is no support for
 * scientific notation, no Blueprint support in Unreal Engine 4, and frequent data loss. FAGX_Real
 * solves the data loss problem with FAGX_RealInterface, an INumericTypeInterface; the scientific
 * notation limitation with FAGX_RealDetails, an IPropertyTypeCustomization; and the Blueprint
 * support with UAGX_Real_FL, a UBlueprintFunctionLibrary.
 *
 * All UClass subclasses that contain an FAGX_Real property also provided BlueprintCallable
 * functions using float since working with FAGX_Real in Blueprints is a bit tedious, but it is
 * still possible to directly manipulate the FAGX_Real value when necessary through the Blueprint
 * Function Library.
 */
USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FAGX_Real
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Dynamics")
	double Value = 0.0;
	// To follow Unreal Engine convention, we may choose to not initialize Value and instead
	// provide a constructor taking an EForceInit and only do value initialization in that
	// constructor. But I like my variables to be initialized.

	FAGX_Real()
	{
	}

	FAGX_Real(double InValue)
		: Value(InValue)
	{
	}

	void SetValue(double InValue)
	{
		Value = InValue;
	}

	double GetValue() const
	{
		return Value;
	}

	double& GetValue()
	{
		return Value;
	}

	operator double() const
	{
		return Value;
	}

	operator double&()
	{
		return Value;
	}

	FAGX_Real& operator=(double InValue)
	{
		Value = InValue;
		return *this;
	}

	/// Called by Unreal Engine when de-serializing an FAGX_Real but some other type was found in
	/// the archive. Floats and doubles are read but all other types are rejected.
	bool SerializeFromMismatchedTag(struct FPropertyTag const& Tag, FStructuredArchive::FSlot Slot);

	// Blueprint Library Functions are in AGX_RealBlueprintFunctionLibrary.h in the AGXUnreal module
	// since we cannot have Unreal Engine stuff in the Barrier module since we aren't allowed to
	// include UObject headers since the Barrier module is built with RTTI while the Unreal Engine
	// modules without RTTI. So we get linker errors on typeinfo
};

/**
 * A struct that informs Unreal Engine about how FAGX_Real can be used.
 */
template <>
struct TStructOpsTypeTraits<FAGX_Real> : public TStructOpsTypeTraitsBase2<FAGX_Real>
{
	// clang-format off
	enum
	{
		// This is the subset of flags from TSTructOpsTypeTraits that we care about for FAGX_Real.
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
