// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

UENUM()
enum class EOpenPLX_InputType : uint8
{
	Unsupported UMETA(DisplayName = "Unsupported"),

	/* Real Inputs */
	AngleInput UMETA(DisplayName = "Angle Input [Real]"),
	AngularVelocity1DInput UMETA(DisplayName = "Angular Velocity 1D Input [Real]"),
	DurationInput UMETA(DisplayName = "Duration Input [Real]"),
	AutomaticClutchEngagementDurationInput UMETA(
		DisplayName =
			"Automatic Clutch Engagement Duration Input [Real]"), // Child of DurationInput
	AutomaticClutchDisengagementDurationInput UMETA(
		DisplayName =
			"Automatic Clutch Disengagement Duration Input [Real]"), // Child of DurationInput
	FractionInput UMETA(DisplayName = "Fraction Input [Real]"),
	Force1DInput UMETA(DisplayName = "Force 1D Input [Real]"),
	LinearVelocity1DInput UMETA(DisplayName = "Linear Velocity 1D Input [Real]"),
	Position1DInput UMETA(DisplayName = "Position 1D Input [Real]"),
	Torque1DInput UMETA(DisplayName = "Torque 1D Input [Real]"),

	/* Range Real Inputs */
	ForceRangeInput UMETA(DisplayName = "Force Range Input [Range Real]"),
	TorqueRangeInput UMETA(DisplayName = "Torque Range Input [Range Real]"),

	/* Vector Inputs */
	AngularVelocity3DInput UMETA(DisplayName = "Angular Velocity 3D Input [Vector]"),
	LinearVelocity3DInput UMETA(DisplayName = "Linear Velocity 3D Input [Vector]"),

	/* Integer Inputs */
	IntInput UMETA(DisplayName = "Int Input [Integer]"),

	/* Boolean Inputs */
	BoolInput UMETA(DisplayName = "Bool Input [Boolean]"),
	ActivateInput UMETA(DisplayName = "Activate Input [Boolean]"), // Child of BoolInput
	EnableInteractionInput UMETA(
		DisplayName = "Enable Interaction Input [Boolean]"), // Child of BoolInput
	EngageInput UMETA(DisplayName = "Engage Input [Boolean]"), // Child of BoolInput
	TorqueConverterLockUpInput UMETA(
		DisplayName = "Torque Converter Lock Up Input [Boolean]") // Child of BoolInput
};

UENUM()
enum class EOpenPLX_OutputType : uint8
{
	Unsupported UMETA(DisplayName = "Unsupported"),

	/* Real Outputs */
	AngleOutput UMETA(DisplayName = "Angle Output [Real]"),
	AngularVelocity1DOutput UMETA(DisplayName = "Angular Velocity 1D Output [Real]"),
	DurationOutput UMETA(DisplayName = "Duration Output [Real]"),
	AutomaticClutchEngagementDurationOutput UMETA(
		DisplayName =
			"Automatic Clutch Engagement Duration Output [Real]"), // Child of DurationOutput
	AutomaticClutchDisengagementDurationOutput UMETA(
		DisplayName =
			"Automatic Clutch Disengagement Duration Output [Real]"), // Child of DurationOutput
	FractionOutput UMETA(DisplayName = "Fraction Output [Real]"),
	Force1DOutput UMETA(DisplayName = "Force 1D Output [Real]"),
	LinearVelocity1DOutput UMETA(DisplayName = "Linear Velocity 1D Output [Real]"),
	MassOutput UMETA(DisplayName = "Mass Output [Real]"),
	Position1DOutput UMETA(DisplayName = "Position 1D Output [Real]"),
	RatioOutput UMETA(DisplayName = "Ratio Output [Real]"),
	RelativeVelocity1DOutput UMETA(DisplayName = "Relative Velocity 1D Output [Real]"),
	RpmOutput UMETA(DisplayName = "RPM Output [Real]"),
	Torque1DOutput UMETA(DisplayName = "Torque 1D Output [Real]"),
	TorqueConverterPumpTorqueOutput UMETA(
		DisplayName = "Torque Converter Pump Torque Output [Real]"),
	TorqueConverterTurbineTorqueOutput UMETA(
		DisplayName = "Torque Converter Turbine Torque Output [Real]"),

	/* Range Real Outputs */
	ForceRangeOutput UMETA(DisplayName = "Force Range Output [Range Real]"),
	TorqueRangeOutput UMETA(DisplayName = "Torque Range Output [Range Real]"),

	/* Vector Outputs */
	AngularVelocity3DOutput UMETA(DisplayName = "Angular Velocity 3D Output [Vector]"),
	Force3DOutput UMETA(DisplayName = "Force 3D Output [Vector]"),
	LinearVelocity3DOutput UMETA(DisplayName = "Linear Velocity 3D Output [Vector]"),
	MateConnectorAcceleration3DOutput UMETA(
		DisplayName = "Mate Connector Acceleration 3D Output [Vector]"),
	MateConnectorAngularAcceleration3DOutput UMETA(
		DisplayName = "Mate Connector Angular Acceleration 3D Output [Vector]"),
	MateConnectorPositionOutput UMETA(DisplayName = "Mate Connector Position Output [Vector]"),
	MateConnectorRPYOutput UMETA(DisplayName = "Mate Connector RPY Output [Vector]"),
	Position3DOutput UMETA(DisplayName = "Position 3D Output [Vector]"),
	RPYOutput UMETA(DisplayName = "RPY Output [Vector]"),
	Torque3DOutput UMETA(DisplayName = "Torque 3D Output [Vector]"),

	/* Integer Outputs */
	IntOutput UMETA(DisplayName = "Int Output [Integer]"),

	/* Boolean Outputs */
	BoolOutput UMETA(DisplayName = "Bool Output [Boolean]"),
	ActivatedOutput UMETA(DisplayName = "Activated Output [Boolean]"), // Child of BoolOutput
	InteractionEnabledOutput UMETA(
		DisplayName = "Interaction Enabled Output [Boolean]"), // Child of BoolOutput
	EngagedOutput UMETA(DisplayName = "Engaged Output [Boolean]"), // Child of BoolOutput
	TorqueConverterLockedUpOutput UMETA(
		DisplayName = "Torque Converter Locked Up Output [Boolean]") // Child of BoolOutput
};
