// Copyright 2025, Algoryx Simulation AB.

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
		case EOpenPLX_OutputType::RelativeVelocity1DOutput:
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
