// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "OpenPLX/OpenPLX_Inputs.h"
#include "OpenPLX/OpenPLX_Outputs.h"

// AGX Dynamics includes.
#include <BeginAGXIncludes.h>
#include <agx/Vec2.h>
#include <agx/Vec3.h>
#include <EndAGXIncludes.h>

// Standard library includes.
#include <cstdint>

/** Shared signal type, unit and coordinate conversions. Native types stay inside the barrier. */
class FOpenPLXSignalConversions
{
public:
	static TOptional<double> ConvertRealToPLX(const FOpenPLX_Input& Input, double Value);

	static TOptional<double> ConvertRealToUnreal(const FOpenPLX_Output& Output, double Value);

	static TOptional<agx::Vec2> ConvertVector2ToPLXValue(
		const FOpenPLX_Input& Input, const FVector2D& Value);

	static TOptional<FVector2D> ConvertVector2ValueToUnreal(
		const FOpenPLX_Output& Output, const agx::Vec2& Value);

	static TOptional<agx::Vec3> ConvertVector3ToPLXValue(
		const FOpenPLX_Input& Input, const FVector& Vector);

	static TOptional<FVector> ConvertVector3ValueToUnreal(
		const FOpenPLX_Output& Output, const agx::Vec3& Value);

	static TOptional<int64_t> ConvertIntegerToPLX(const FOpenPLX_Input& Input, int64 Value);

	static TOptional<int64> ConvertIntegerToUnreal(const FOpenPLX_Output& Output, int64_t Value);

	static TOptional<bool> ConvertBooleanToPLX(const FOpenPLX_Input& Input, bool Value);

	static TOptional<bool> ConvertBooleanToUnreal(const FOpenPLX_Output& Output, bool Value);

	/** Convert an incoming control input using the same inverse rules as model outputs. */
	static TOptional<double> ConvertRealInputToUnreal(const FOpenPLX_Input& Input, double Value);
	static TOptional<FVector2D> ConvertVector2InputToUnreal(
		const FOpenPLX_Input& Input, const agx::Vec2& Value);
	static TOptional<FVector> ConvertVector3InputToUnreal(
		const FOpenPLX_Input& Input, const agx::Vec3& Value);
	static TOptional<int64> ConvertIntegerInputToUnreal(const FOpenPLX_Input& Input, int64_t Value);
	static TOptional<bool> ConvertBooleanInputToUnreal(const FOpenPLX_Input& Input, bool Value);
};
