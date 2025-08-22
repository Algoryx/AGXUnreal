// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Error: 'USTRUCT' must not be inside preprocessor blocks, except for WITH_EDITORONLY_DATA
// #if AGXUNREAL_USE_OPENPLX
#if 0

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

UENUM()
enum class EOpenPLX_InputType : uint8
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
	/* Range Real Inputs */
	ForceRangeInput,
	TorqueRangeInput,
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
enum class EOpenPLX_OutputType : uint8
{
	Unsupported,
	/* Real Outputs */
	AngleOutput,
	AngularVelocity1DOutput,
	DurationOutput,
	AutomaticClutchEngagementDurationOutput, // Child of DurationOutput
	AutomaticClutchDisengagementDurationOutput, // Child of DurationOutput
	FractionOutput,
	Force1DOutput,
	LinearVelocity1DOutput,
	Position1DOutput,
	RelativeVelocity1DOutput,
	Torque1DOutput,
	TorqueConverterPumpTorqueOutput,
	TorqueConverterTurbineTorqueOutput,
	/* Range Real Outputs */
	ForceRangeOutput,
	TorqueRangeOutput,
	/* Vec3 Real Outputs */
	AngularVelocity3DOutput,
	Force3DOutput,
	LinearVelocity3DOutput,
	MateConnectorAcceleration3DOutput,
	MateConnectorAngularAcceleration3DOutput,
	MateConnectorPositionOutput,
	MateConnectorRPYOutput,
	Position3DOutput,
	RPYOutput,
	Torque3DOutput,
	/* Integer Outputs */
	IntOutput,
	/* Boolean Outputs */
	BoolOutput,
	ActivatedOutput, // Child of BoolOutput
	InteractionEnabledOutput, // Child of BoolOutput
	EngagedOutput, // Child of BoolOutput
	TorqueConverterLockedUpOutput, // Child of BoolOutput
};


#endif
