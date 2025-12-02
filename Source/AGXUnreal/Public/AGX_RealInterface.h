// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"

// Unreal Engine includes.
#include "Widgets/Input/NumericTypeInterface.h"

/**
 * A INumericTypeInterface is responsible for converting between string representations and
 * numeric representations in a widget. This one has support for scientific notation and infinity.
 */
class AGXUNREAL_API FAGX_RealInterface : public INumericTypeInterface<double>
{
public: // Helper functions.
	/// Public utility function to convert a number to a string.
	static FString StaticToString(const double& Value);

	/// Public utility function to convert a string to a number. Will ignore invalid characters
	/// after the number.
	static TOptional<double> StaticFromString(const FString& InString);

public: // Member function overrides.
	//~ Begin INumericTypeInterface.
	virtual FString ToString(const double& Value) const override;
	virtual TOptional<double> FromString(const FString& InString, const double&) override;
	virtual bool IsCharacterValid(TCHAR InChar) const override;
	// Features not supported by this implementation
	virtual int32 GetMinFractionalDigits() const override;
	virtual int32 GetMaxFractionalDigits() const override;
	virtual void SetMinFractionalDigits(const TAttribute<TOptional<int32>>& NewValue) override;
	virtual void SetMaxFractionalDigits(const TAttribute<TOptional<int32>>& NewValue) override;
	//~ End INumericTypeInterface.
};
