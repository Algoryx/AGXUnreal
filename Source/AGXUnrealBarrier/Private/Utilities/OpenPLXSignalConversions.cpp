// Copyright 2026, Algoryx Simulation AB.

#include "Utilities/OpenPLXSignalConversions.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "BarrierOnly/AGXTypeConversions.h"

//
// Here follows a bunch of type conversion functions that convert OpenPLX typed values to and
// from Unreal typed values. The functions ensure that Input or Output type makes sense for the
// C++ type of the value and performs appropriate unit conversion where necessary.
//

//
// Conversion functions for Real.
//

TOptional<double> FOpenPLXSignalConversions::ConvertRealToPLX(
	const FOpenPLX_Input& Input, double Value)
{
	switch (Input.Type)
	{
		case EOpenPLX_InputType::DurationInput:
		case EOpenPLX_InputType::AutomaticClutchEngagementDurationInput:
		case EOpenPLX_InputType::AutomaticClutchDisengagementDurationInput:
		case EOpenPLX_InputType::FractionInput:
		case EOpenPLX_InputType::Force1DInput:
		case EOpenPLX_InputType::Torque1DInput:
			return Value;
		case EOpenPLX_InputType::AngleInput:
		case EOpenPLX_InputType::AngularVelocity1DInput:
			return ConvertAngleToAGX(Value);
		case EOpenPLX_InputType::Position1DInput:
		case EOpenPLX_InputType::LinearVelocity1DInput:
			return ConvertDistanceToAGX(Value);
	}

	UE_LOG(
		LogAGX, Warning,
		TEXT(
			"OpenPLX Signal Conversions: Tried to convert Real value for Input '%s' ('%s') to the "
			"corresponding OpenPLX value, but the Input is either not of Real type or is "
			"unsupported."),
		*Input.Name.ToString(), *Input.Alias.ToString());
	return {};
}

TOptional<double> FOpenPLXSignalConversions::ConvertRealToUnreal(
	const FOpenPLX_Output& Output, double Value)
{
	switch (Output.Type)
	{
		case EOpenPLX_OutputType::DurationOutput:
		case EOpenPLX_OutputType::AutomaticClutchEngagementDurationOutput:
		case EOpenPLX_OutputType::AutomaticClutchDisengagementDurationOutput:
		case EOpenPLX_OutputType::FractionOutput:
		case EOpenPLX_OutputType::Force1DOutput:
		case EOpenPLX_OutputType::MassOutput:
		case EOpenPLX_OutputType::RatioOutput:
		case EOpenPLX_OutputType::RpmOutput:
		case EOpenPLX_OutputType::Torque1DOutput:
		case EOpenPLX_OutputType::TorqueConverterPumpTorqueOutput:
		case EOpenPLX_OutputType::TorqueConverterTurbineTorqueOutput:
			return Value;
		case EOpenPLX_OutputType::AngleOutput:
		case EOpenPLX_OutputType::AngularVelocity1DOutput:
			return ConvertAngleToUnreal<double>(Value);
		case EOpenPLX_OutputType::LinearVelocity1DOutput:
		case EOpenPLX_OutputType::Position1DOutput:
		case EOpenPLX_OutputType::RelativeVelocity1DOutput:
			return ConvertDistanceToUnreal<double>(Value);
	}

	UE_LOG(
		LogAGX, Warning,
		TEXT(
			"OpenPLX Signal Conversions: Tried to convert Real value for Output '%s' ('%s') to the "
			"corresponding Unreal value, but the Output is either not of Real type or "
			"is unsupported."),
		*Output.Name.ToString(), *Output.Alias.ToString());
	return {};
}

//
// Conversion functions for Vec2.
//

TOptional<agx::Vec2> FOpenPLXSignalConversions::ConvertVector2ToPLXValue(
	const FOpenPLX_Input& Input, const FVector2D& Value)
{
	switch (Input.Type)
	{
		case EOpenPLX_InputType::ForceRangeInput:
		case EOpenPLX_InputType::TorqueRangeInput:
			return {{Value.X, Value.Y}};
	}

	UE_LOG(
		LogAGX, Warning,
		TEXT(
			"OpenPLX Signal Conversions: Tried to convert Vector2 value for Input '%s' ('%s'), to "
			"the corresponding OpenPLX value, but the Input is either not of Vector2 type or "
			"is unsupported."),
		*Input.Name.ToString(), *Input.Alias.ToString());
	return {};
}

TOptional<FVector2D> FOpenPLXSignalConversions::ConvertVector2ValueToUnreal(
	const FOpenPLX_Output& Output, const agx::Vec2& Value)
{
	switch (Output.Type)
	{
		case EOpenPLX_OutputType::ForceRangeOutput:
		case EOpenPLX_OutputType::TorqueRangeOutput:
			return {{Value.x(), Value.y()}};
	}

	UE_LOG(
		LogAGX, Warning,
		TEXT(
			"OpenPLX Signal Conversions: Tried to convert Vector2 value for Output '%s' ('%s'), to "
			"the corresponding Unreal value, but Output is either not of Vector2 type or is "
			"unsupported."),
		*Output.Name.ToString(), *Output.Alias.ToString());
	return {};
}

//
// Conversion functions for Vec3.
//

TOptional<agx::Vec3> FOpenPLXSignalConversions::ConvertVector3ToPLXValue(
	const FOpenPLX_Input& Input, const FVector& Vector)
{
	switch (Input.Type)
	{
		case EOpenPLX_InputType::AngularVelocity3DInput:
			return ConvertAngularVelocity(Vector);
		case EOpenPLX_InputType::LinearVelocity3DInput:
			return ConvertDisplacement(Vector);
		case EOpenPLX_InputType::Torque3DInput:
			return ConvertTorque(Vector);
	}

	UE_LOG(
		LogAGX, Warning,
		TEXT(
			"OpenPLX Signal Conversions: Tried to convert Vector value for Input '%s' ('%s'), to "
			"the corresponding OpenPLX value, but the Input is either not of Vector type or "
			"is unsupported."),
		*Input.Name.ToString(), *Input.Alias.ToString());
	return {};
}

TOptional<FVector> FOpenPLXSignalConversions::ConvertVector3ValueToUnreal(
	const FOpenPLX_Output& Output, const agx::Vec3& Value)
{
	switch (Output.Type)
	{
		case EOpenPLX_OutputType::AngularVelocity3DOutput:
		case EOpenPLX_OutputType::MateConnectorAngularAcceleration3DOutput:
		case EOpenPLX_OutputType::MateConnectorAngularVelocity3DOutput:
			return ConvertAngularVelocity(Value);
		case EOpenPLX_OutputType::MateConnectorRPYOutput:
		case EOpenPLX_OutputType::RPYOutput:
			return ConvertRPY(Value);
		case EOpenPLX_OutputType::LinearVelocity3DOutput:
		case EOpenPLX_OutputType::MateConnectorAcceleration3DOutput:
		case EOpenPLX_OutputType::MateConnectorLinearVelocity3DOutput:
		case EOpenPLX_OutputType::MateConnectorPositionOutput:
		case EOpenPLX_OutputType::Position3DOutput:
			return ConvertDisplacement(Value);
		case EOpenPLX_OutputType::Force3DOutput:
			return ConvertVector(Value);
		case EOpenPLX_OutputType::Torque3DOutput:
			return ConvertTorque(Value);
	}

	UE_LOG(
		LogAGX, Warning,
		TEXT(
			"OpenPLX Signal Conversions: Tried to convert Vector value Output '%s' ('%s') to the "
			"corresponding Unreal value, but the Output is either not of Vector type or is "
			"unsupported."),
		*Output.Name.ToString(), *Output.Alias.ToString());
	return {};
}

//
// Conversion functions for Integer.
//

TOptional<int64_t> FOpenPLXSignalConversions::ConvertIntegerToPLX(
	const FOpenPLX_Input& Input, int64 Value)
{
	switch (Input.Type)
	{
		case EOpenPLX_InputType::IntInput:
			return {static_cast<int64_t>(Value)};
	}

	UE_LOG(
		LogAGX, Warning,
		TEXT(
			"OpenPLX Signal Conversions: Tried to convert Integer value for Input '%s' ('%s') to "
			"the corresponding OpenPLX value, but the Input is either not of integer type or "
			"is unsupported."),
		*Input.Name.ToString(), *Input.Alias.ToString());
	return {};
}

TOptional<int64> FOpenPLXSignalConversions::ConvertIntegerToUnreal(
	const FOpenPLX_Output& Output, int64_t Value)
{
	switch (Output.Type)
	{
		case EOpenPLX_OutputType::IntOutput:
			return {static_cast<int64>(Value)};
	}

	UE_LOG(
		LogAGX, Warning,
		TEXT(
			"OpenPLX Signal Conversions: Tried to convert Integer value for Output '%s' ('%s') to"
			"the corresponding Unreal value, but the Output is either not of Integer type or "
			"is unsupported."),
		*Output.Name.ToString(), *Output.Alias.ToString());
	return {};
}

//
// Conversion functions for Boolean.
//

TOptional<bool> FOpenPLXSignalConversions::ConvertBooleanToPLX(
	const FOpenPLX_Input& Input, bool Value)
{
	switch (Input.Type)
	{
		case EOpenPLX_InputType::BoolInput:
		case EOpenPLX_InputType::ActivateInput:
		case EOpenPLX_InputType::EnableInteractionInput:
		case EOpenPLX_InputType::EngageInput:
		case EOpenPLX_InputType::TorqueConverterLockUpInput:
			return Value;
	}

	UE_LOG(
		LogAGX, Warning,
		TEXT(
			"OpenPLX Signal Conversions: Tried to convert Boolean value for Input '%s' ('%s') to "
			"the corresponding OpenPLX value, but the Input is either not of Boolean type or "
			"is unsupported."),
		*Input.Name.ToString(), *Input.Alias.ToString());
	return {};
}

TOptional<bool> FOpenPLXSignalConversions::ConvertBooleanToUnreal(
	const FOpenPLX_Output& Output, bool Value)
{
	switch (Output.Type)
	{
		case EOpenPLX_OutputType::BoolOutput:
		case EOpenPLX_OutputType::ActivatedOutput:
		case EOpenPLX_OutputType::InteractionEnabledOutput:
		case EOpenPLX_OutputType::EngagedOutput:
		case EOpenPLX_OutputType::TorqueConverterLockedUpOutput:
			return Value;
	}

	UE_LOG(
		LogAGX, Warning,
		TEXT(
			"OpenPLX Signal Conversions: Tried to convert Boolean value for Output '%s' ('%s') to "
			"the corresponding Unreal value, but the Output is either not of Boolean type or "
			"is unsupported."),
		*Output.Name.ToString(), *Output.Alias.ToString());
	return {};
}

namespace
{
	// Inputs and their corresponding outputs share physical units and coordinate conventions.
	// This descriptor selects existing inverse conversion rules; it does not read a model output.
	FOpenPLX_Output GetConversionOutput(const FOpenPLX_Input& Input)
	{
		EOpenPLX_OutputType Type = EOpenPLX_OutputType::Unsupported;
		switch (Input.Type)
		{
			case EOpenPLX_InputType::DurationInput:
				Type = EOpenPLX_OutputType::DurationOutput;
				break;
			case EOpenPLX_InputType::AutomaticClutchEngagementDurationInput:
				Type = EOpenPLX_OutputType::AutomaticClutchEngagementDurationOutput;
				break;
			case EOpenPLX_InputType::AutomaticClutchDisengagementDurationInput:
				Type = EOpenPLX_OutputType::AutomaticClutchDisengagementDurationOutput;
				break;
			case EOpenPLX_InputType::FractionInput:
				Type = EOpenPLX_OutputType::FractionOutput;
				break;
			case EOpenPLX_InputType::Force1DInput:
				Type = EOpenPLX_OutputType::Force1DOutput;
				break;
			case EOpenPLX_InputType::Torque1DInput:
				Type = EOpenPLX_OutputType::Torque1DOutput;
				break;
			case EOpenPLX_InputType::AngleInput:
				Type = EOpenPLX_OutputType::AngleOutput;
				break;
			case EOpenPLX_InputType::AngularVelocity1DInput:
				Type = EOpenPLX_OutputType::AngularVelocity1DOutput;
				break;
			case EOpenPLX_InputType::Position1DInput:
				Type = EOpenPLX_OutputType::Position1DOutput;
				break;
			case EOpenPLX_InputType::LinearVelocity1DInput:
				Type = EOpenPLX_OutputType::LinearVelocity1DOutput;
				break;
			case EOpenPLX_InputType::ForceRangeInput:
				Type = EOpenPLX_OutputType::ForceRangeOutput;
				break;
			case EOpenPLX_InputType::TorqueRangeInput:
				Type = EOpenPLX_OutputType::TorqueRangeOutput;
				break;
			case EOpenPLX_InputType::AngularVelocity3DInput:
				Type = EOpenPLX_OutputType::AngularVelocity3DOutput;
				break;
			case EOpenPLX_InputType::LinearVelocity3DInput:
				Type = EOpenPLX_OutputType::LinearVelocity3DOutput;
				break;
			case EOpenPLX_InputType::Torque3DInput:
				Type = EOpenPLX_OutputType::Torque3DOutput;
				break;
			case EOpenPLX_InputType::IntInput:
				Type = EOpenPLX_OutputType::IntOutput;
				break;
			case EOpenPLX_InputType::BoolInput:
				Type = EOpenPLX_OutputType::BoolOutput;
				break;
			case EOpenPLX_InputType::ActivateInput:
				Type = EOpenPLX_OutputType::ActivatedOutput;
				break;
			case EOpenPLX_InputType::EnableInteractionInput:
				Type = EOpenPLX_OutputType::InteractionEnabledOutput;
				break;
			case EOpenPLX_InputType::EngageInput:
				Type = EOpenPLX_OutputType::EngagedOutput;
				break;
			case EOpenPLX_InputType::TorqueConverterLockUpInput:
				Type = EOpenPLX_OutputType::TorqueConverterLockedUpOutput;
				break;
		}
		return FOpenPLX_Output(Input.Name, Input.Alias, Type, true);
	}
}

TOptional<double> FOpenPLXSignalConversions::ConvertRealInputToUnreal(
	const FOpenPLX_Input& Input, double Value)
{
	return ConvertRealToUnreal(GetConversionOutput(Input), Value);
}

TOptional<FVector2D> FOpenPLXSignalConversions::ConvertVector2InputToUnreal(
	const FOpenPLX_Input& Input, const agx::Vec2& Value)
{
	return ConvertVector2ValueToUnreal(GetConversionOutput(Input), Value);
}

TOptional<FVector> FOpenPLXSignalConversions::ConvertVector3InputToUnreal(
	const FOpenPLX_Input& Input, const agx::Vec3& Value)
{
	return ConvertVector3ValueToUnreal(GetConversionOutput(Input), Value);
}

TOptional<int64> FOpenPLXSignalConversions::ConvertIntegerInputToUnreal(
	const FOpenPLX_Input& Input, int64_t Value)
{
	return ConvertIntegerToUnreal(GetConversionOutput(Input), Value);
}

TOptional<bool> FOpenPLXSignalConversions::ConvertBooleanInputToUnreal(
	const FOpenPLX_Input& Input, bool Value)
{
	return ConvertBooleanToUnreal(GetConversionOutput(Input), Value);
}
