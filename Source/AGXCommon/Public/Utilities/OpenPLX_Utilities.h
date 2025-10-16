// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "OpenPLX/OpenPLX_Enums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"


class AGXCOMMON_API FOpenPLX_Utilities
{
public:
	static bool IsRealType(EOpenPLX_InputType Type);
	static bool IsRealType(EOpenPLX_OutputType Type);

	static bool IsRangeType(EOpenPLX_InputType Type);
	static bool IsRangeType(EOpenPLX_OutputType Type);

	static bool IsVectorType(EOpenPLX_InputType Type);
	static bool IsVectorType(EOpenPLX_OutputType Type);

	static bool IsIntegerType(EOpenPLX_InputType Type);
	static bool IsIntegerType(EOpenPLX_OutputType Type);

	static bool IsBooleanType(EOpenPLX_InputType Type);
	static bool IsBooleanType(EOpenPLX_OutputType Type);
};
