// Copyright 2025, Algoryx Simulation AB.

#include "AGX_RealInterface.h"

FString FAGX_RealInterface::StaticToString(const double& Value)
{
	FString Result = FString::Printf(TEXT("%g"), Value);
	return Result;
}

TOptional<double> FAGX_RealInterface::StaticFromString(const FString& InString)
{
	// Unfortunately, FCString::Atod cannot detect parse errors. It's return type is double,
	// not TOptional<double>, and on error it returns 0.0, which is a common value in
	// real-world data. Therefore Result.IsSet() will always evaluate to true. The result of
	// that is when the users enters an invalid string the FAGX_Real will be set to 0.0.
	//
	// If we want to detect these error cases in order to restore/keep the old value then we
	// either need to do input validation ourselves here or use another string-to-double
	// conversion function. I tried with std::stod, which can report conversion errors, but
	// it does it via exceptions and I'm not sure Unreal Engine like exceptions.
	TOptional<double> Result = FCString::Atod(*InString);
	if (!Result.IsSet())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("FAGX_Real tried to convert string '%s' to double, but Atod failed."), *InString);
	}
	return Result;
}

FString FAGX_RealInterface::ToString(const double& Value) const
{
	return StaticToString(Value);
}

TOptional<double> FAGX_RealInterface::FromString(const FString& InString, const double&)
{
	return StaticFromString(InString);
}

bool FAGX_RealInterface::IsCharacterValid(TCHAR InChar) const
{
	auto IsValidLocalizedCharacter = [InChar]() -> bool
	{
		const FDecimalNumberFormattingRules& NumberFormattingRules =
			ExpressionParser::GetLocalizedNumberFormattingRules();
		return InChar == NumberFormattingRules.GroupingSeparatorCharacter ||
			   InChar == NumberFormattingRules.DecimalSeparatorCharacter ||
			   Algo::Find(NumberFormattingRules.DigitCharacters, InChar) != 0;
	};

	static const FString ValidChars = TEXT("1234567890eEiInNfF-+.");
	return InChar != 0 &&
		   (ValidChars.GetCharArray().Contains(InChar) || IsValidLocalizedCharacter());
}

int32 FAGX_RealInterface::GetMinFractionalDigits() const
{
	return 0;
}

/// Max Fractional Digits is not used by this Numeric Type Interface.
int32 FAGX_RealInterface::GetMaxFractionalDigits() const
{
	return 0;
}

/// Min Fractional Digits is not used by this Numeric Type Interface.
void FAGX_RealInterface::SetMinFractionalDigits(const TAttribute<TOptional<int32>>& NewValue)
{
}

/// Max Fractional Digits is not used by this Numeric Type Interface.
void FAGX_RealInterface::SetMaxFractionalDigits(const TAttribute<TOptional<int32>>& NewValue)
{
}
