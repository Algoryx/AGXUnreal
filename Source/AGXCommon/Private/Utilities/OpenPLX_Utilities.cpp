// Copyright 2026, Algoryx Simulation AB.

#include "Utilities/OpenPLX_Utilities.h"

bool FOpenPLX_Utilities::IsRealType(EOpenPLX_InputType Type)
{
	switch (Type)
	{
		case EOpenPLX_InputType::AngleInput:
		case EOpenPLX_InputType::AngularVelocity1DInput:
		case EOpenPLX_InputType::DurationInput:
		case EOpenPLX_InputType::AutomaticClutchEngagementDurationInput:
		case EOpenPLX_InputType::AutomaticClutchDisengagementDurationInput:
		case EOpenPLX_InputType::FractionInput:
		case EOpenPLX_InputType::Force1DInput:
		case EOpenPLX_InputType::LinearVelocity1DInput:
		case EOpenPLX_InputType::Position1DInput:
		case EOpenPLX_InputType::Torque1DInput:
			return true;
		default:
			return false;
	}
}

bool FOpenPLX_Utilities::IsRealType(EOpenPLX_OutputType Type)
{
	switch (Type)
	{
		case EOpenPLX_OutputType::AngleOutput:
		case EOpenPLX_OutputType::AngularVelocity1DOutput:
		case EOpenPLX_OutputType::DurationOutput:
		case EOpenPLX_OutputType::AutomaticClutchEngagementDurationOutput:
		case EOpenPLX_OutputType::AutomaticClutchDisengagementDurationOutput:
		case EOpenPLX_OutputType::FractionOutput:
		case EOpenPLX_OutputType::Force1DOutput:
		case EOpenPLX_OutputType::LinearVelocity1DOutput:
		case EOpenPLX_OutputType::MassOutput:
		case EOpenPLX_OutputType::Position1DOutput:
		case EOpenPLX_OutputType::RatioOutput:
		case EOpenPLX_OutputType::RelativeVelocity1DOutput:
		case EOpenPLX_OutputType::RpmOutput:
		case EOpenPLX_OutputType::Torque1DOutput:
		case EOpenPLX_OutputType::TorqueConverterPumpTorqueOutput:
		case EOpenPLX_OutputType::TorqueConverterTurbineTorqueOutput:
			return true;
		default:
			return false;
	}
}

bool FOpenPLX_Utilities::IsRangeType(EOpenPLX_InputType Type)
{
	switch (Type)
	{
		case EOpenPLX_InputType::ForceRangeInput:
		case EOpenPLX_InputType::TorqueRangeInput:
			return true;
		default:
			return false;
	}
}

bool FOpenPLX_Utilities::IsRangeType(EOpenPLX_OutputType Type)
{
	switch (Type)
	{
		case EOpenPLX_OutputType::ForceRangeOutput:
		case EOpenPLX_OutputType::TorqueRangeOutput:
			return true;
		default:
			return false;
	}
}

bool FOpenPLX_Utilities::IsVector2Type(EOpenPLX_InputType Type)
{
	// Add switch statement here once we identify an input with Vector2 type.
	return false;
}

bool FOpenPLX_Utilities::IsVector2Type(EOpenPLX_OutputType Type)
{
	// Add switch statement here once we identify an output with Vector2 type.
	return false;
}

bool FOpenPLX_Utilities::IsVectorType(EOpenPLX_InputType Type)
{
	switch (Type)
	{
		case EOpenPLX_InputType::AngularVelocity3DInput:
		case EOpenPLX_InputType::LinearVelocity3DInput:
			return true;
		default:
			return false;
	}
}

bool FOpenPLX_Utilities::IsVectorType(EOpenPLX_OutputType Type)
{
	switch (Type)
	{
		case EOpenPLX_OutputType::AngularVelocity3DOutput:
		case EOpenPLX_OutputType::Force3DOutput:
		case EOpenPLX_OutputType::LinearVelocity3DOutput:
		case EOpenPLX_OutputType::MateConnectorAcceleration3DOutput:
		case EOpenPLX_OutputType::MateConnectorAngularAcceleration3DOutput:
		case EOpenPLX_OutputType::MateConnectorPositionOutput:
		case EOpenPLX_OutputType::MateConnectorRPYOutput:
		case EOpenPLX_OutputType::Position3DOutput:
		case EOpenPLX_OutputType::RPYOutput:
		case EOpenPLX_OutputType::Torque3DOutput:
			return true;
		default:
			return false;
	}
}

bool FOpenPLX_Utilities::IsIntegerType(EOpenPLX_InputType Type)
{
	switch (Type)
	{
		case EOpenPLX_InputType::IntInput:
			return true;
		default:
			return false;
	}
}

bool FOpenPLX_Utilities::IsIntegerType(EOpenPLX_OutputType Type)
{
	switch (Type)
	{
		case EOpenPLX_OutputType::IntOutput:
			return true;
		default:
			return false;
	}
}

bool FOpenPLX_Utilities::IsUnsignedIntegerType(EOpenPLX_InputType Type)
{
	// Add switch statement here once we identify an input with unsigned integer type.
	return false;
}

bool FOpenPLX_Utilities::IsUnsignedIntegerType(EOpenPLX_OutputType Type)
{
	// Add switch statement here once we identify an output with unsigned integer type.
	return false;
}

bool FOpenPLX_Utilities::IsBooleanType(EOpenPLX_InputType Type)
{
	switch (Type)
	{
		case EOpenPLX_InputType::BoolInput:
		case EOpenPLX_InputType::ActivateInput:
		case EOpenPLX_InputType::EnableInteractionInput:
		case EOpenPLX_InputType::EngageInput:
		case EOpenPLX_InputType::TorqueConverterLockUpInput:
			return true;
		default:
			return false;
	}
}

bool FOpenPLX_Utilities::IsBooleanType(EOpenPLX_OutputType Type)
{
	switch (Type)
	{
		case EOpenPLX_OutputType::BoolOutput:
		case EOpenPLX_OutputType::ActivatedOutput:
		case EOpenPLX_OutputType::InteractionEnabledOutput:
		case EOpenPLX_OutputType::EngagedOutput:
		case EOpenPLX_OutputType::TorqueConverterLockedUpOutput:
			return true;
		default:
			return false;
	}
}

bool FOpenPLX_Utilities::IsIMUOutputType(EOpenPLX_OutputType Type)
{
	switch (Type)
	{
		case EOpenPLX_OutputType::IMUOutput:
			return true;
		default:
			return false;
	}
}

bool FOpenPLX_Utilities::IsLidarOutputType(EOpenPLX_OutputType Type)
{
	switch (Type)
	{
		case EOpenPLX_OutputType::LidarOutput:
			return true;
		default:
			return false;
	}
}

const TCHAR* FOpenPLX_Utilities::GetPrimitiveTypeName(EOpenPLX_InputType Type)
{
	if (IsBooleanType(Type))
		return TEXT("Boolean");
	if (IsIntegerType(Type))
		return TEXT("Integer");
	if (IsUnsignedIntegerType(Type))
		return TEXT("UnsignedInteger");
	if (IsRealType(Type))
		return TEXT("Real");
	if (IsRangeType(Type))
		return TEXT("RangeReal");
	if (IsVector2Type(Type))
		return TEXT("Vector2");
	if (IsVectorType(Type))
		return TEXT("Vector");
	return TEXT("(unknown)");
}

const TCHAR* FOpenPLX_Utilities::GetPrimitiveTypeName(EOpenPLX_OutputType Type)
{
	if (IsBooleanType(Type))
		return TEXT("Boolean");
	if (IsIntegerType(Type))
		return TEXT("Integer");
	if (IsUnsignedIntegerType(Type))
		return TEXT("UnsignedInteger");
	if (IsRealType(Type))
		return TEXT("Real");
	if (IsRangeType(Type))
		return TEXT("RangeReal");
	if (IsVector2Type(Type))
		return TEXT("Vector2");
	if (IsVectorType(Type))
		return TEXT("Vector");
	if (IsLidarOutputType(Type))
		return TEXT("Lidar Output");
	return TEXT("(unknown)");
}
