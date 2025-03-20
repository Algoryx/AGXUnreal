// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

UENUM()
enum class EPLX_InputType
{
	Unsupported,
	/* Real Inputs */
	AngleInput,
	AngularVelocity1DInput,
	DurationInput,
	AutomaticClutchEngagementDurationInput, // Child of DurationInput
	AutomaticClutchDisengagementDurationInput, // Child of DurationInput
	FractionInput,
	Force1DInput,
	LinearVelocity1DInput,
	Position1DInput,
	Torque1DInput,
	/* Vec3 Real Inputs */
	AngularVelocity3DInput,
	LinearVelocity3DInput,
	/* Integer Inputs */
	IntInput,
	/* Boolean Inputs */
	BoolInput,
	ActivateInput, // Child of BoolInput
	EnableInteractionInput, // Child of BoolInput
	EngageInput, // Child of BoolInput
	TorqueConverterLockUpInput // Child of BoolInput
};

UENUM()
enum class EPLX_OutputType
{
	Unsupported,
	/* Real Outputs */
	Acceleration3DOutput,
	AngleOutput,
	AngularAcceleration3DOutput,
	AngularVelocity1DOutput,
	DurationOutput,
	AutomaticClutchEngagementDurationOutput, // Child of DurationOutput
	AutomaticClutchDisengagementDurationOutput, // Child of DurationOutput
	FractionOutput,
	Force1DOutput,
	LinearVelocity1DOutput,
	Position1DOutput,
	Position3DOutput,
	RelativeVelocity1DOutput,
	RPYOutput,
	Torque1DOutput,
	TorqueConverterPumpTorqueOutput,
	TorqueConverterTurbineTorqueOutput,
	/* Vec3 Real Outputs */
	AngularVelocity3DOutput,
	LinearVelocity3DOutput,
	/* Integer Outputs */
	IntOutput,
	/* Boolean Outputs */
	BoolOutput,
	ActivatedOutput, // Child of BoolOutput
	InteractionEnabledOutput, // Child of BoolOutput
	EngagedOutput, // Child of BoolOutput
	TorqueConverterLockedUpOutput, // Child of BoolOutput
};
