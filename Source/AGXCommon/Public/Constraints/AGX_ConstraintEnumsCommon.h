// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ConstraintEnumsCommon.generated.h"

UENUM(BlueprintType)
enum class EAGX_ConstraintAngleControllerType : uint8
{
	/// Corresponds to agx::RotationalAngle type.
	RotationalAngle,

	/// Corresponds to agx::RotationalAngle type.
	SeparationAngle,
};

UENUM(BlueprintType)
enum class EAGX_ConstraintControllerType : uint8
{
	Invalid,

	/// Corresponds to FAGX_ConstraintTargetSpeedController type.
	ConstraintTargetSpeedController,

	/// Corresponds to FAGX_ConstraintLockController type.
	ConstraintLockController,

	/// Corresponds to FAGX_ConstraintRangeController type.
	ConstraintRangeController,

	/// Corresponds to FAGX_ConstraintElectricMotorController type.
	ConstraintElectricMotorController,

	/// Corresponds to FAGX_ConstraintFrictionController type.
	ConstraintFrictionController

};
