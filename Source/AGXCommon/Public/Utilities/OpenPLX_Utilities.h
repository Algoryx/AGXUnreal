// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "OpenPLX/OpenPLX_Enums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

struct FOpenPLX_Input;
struct FOpenPLX_Output;

class AGXCOMMON_API FOpenPLX_Utilities
{
public:
	static EOpenPLX_ValueLayout GetValueLayout(EOpenPLX_InputType Type);
	static EOpenPLX_ValueLayout GetValueLayout(EOpenPLX_OutputType Type);

	/** Return ValueLayout, or derive it from Type when ValueLayout is Unspecified. */
	static EOpenPLX_ValueLayout GetResolvedValueLayout(const FOpenPLX_Input& Input);
	static EOpenPLX_ValueLayout GetResolvedValueLayout(const FOpenPLX_Output& Output);

	static bool IsRealType(EOpenPLX_InputType Type);
	static bool IsRealType(EOpenPLX_OutputType Type);

	static bool IsRangeType(EOpenPLX_InputType Type);
	static bool IsRangeType(EOpenPLX_OutputType Type);

	static bool IsVector2Type(EOpenPLX_InputType Type);
	static bool IsVector2Type(EOpenPLX_OutputType Type);

	static bool IsRPYType(EOpenPLX_InputType Type);
	static bool IsRPYType(EOpenPLX_OutputType Type);

	static bool IsVectorType(EOpenPLX_InputType Type);
	static bool IsVectorType(EOpenPLX_OutputType Type);

	static bool IsIntegerType(EOpenPLX_InputType Type);
	static bool IsIntegerType(EOpenPLX_OutputType Type);

	static bool IsUnsignedIntegerType(EOpenPLX_InputType Type);
	static bool IsUnsignedIntegerType(EOpenPLX_OutputType Type);

	static bool IsBooleanType(EOpenPLX_InputType Type);
	static bool IsBooleanType(EOpenPLX_OutputType Type);
	
	static bool IsIMUOutputType(EOpenPLX_OutputType Type);
	static bool IsLidarOutputType(EOpenPLX_OutputType Type);

	static const TCHAR* GetPrimitiveTypeName(EOpenPLX_InputType Type);
	static const TCHAR* GetPrimitiveTypeName(EOpenPLX_OutputType Type);
};
