// Copyright 2025, Algoryx Simulation AB.

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

	/// Corresponds to agx::TargetSpeedController type.
	ConstraintTargetSpeedController,

	/// Corresponds to agx::LockController type.
	ConstraintLockController,

	/// Corresponds to agx::RangeController type.
	ConstraintRangeController,

	/// Corresponds to agx::ElectricMotorController type.
	ConstraintElectricMotorController,

	/// Corresponds to agx::FrictionController type.
	ConstraintFrictionController

};
