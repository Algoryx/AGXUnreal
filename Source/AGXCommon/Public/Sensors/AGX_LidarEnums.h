// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

/** Specifies the model of the Lidar. */
UENUM(BlueprintType)
enum class EAGX_LidarModel : uint8
{
	/** Lidar uses a custom ray pattern where the user provides the ray pattern rays. */
	CustomRayPattern,

	/** Generic Lidar with a Horizontal Sweep ray pattern. */
	GenericHorizontalSweep,

	/** Ouster OS0 */
	OusterOS0,

	/** Ouster OS1 */
	OusterOS1,

	/** Ouster OS2 */
	OusterOS2
};

/** Specifies which Components the Lidar Surface Material Component applies the Lidar Surface
 * Material to. */
UENUM(BlueprintType)
enum class EAGX_LidarSurfaceMaterialAssignmentSelection : uint8
{
	/**
	 * Assign the Lidar Surface Material to the parent Component. Searches up the Component hierarcy
	 * for the first valid parent.
	 */
	Parent,

	/**
	 * Assign the Lidar Surface Material to all sibling Components, i.e. all Components that share
	 * the same immediate Parent.
	 */
	Siblings,

	/**
	 * Assign the Lidar Surface Material to all child Components. Searches recursively.
	 */
	Children,
};

/** Number of channels setting for OusterOS Lidars. */
UENUM(BlueprintType)
enum class EAGX_OusterOSChannelCount : uint8
{
	CH_32,
	CH_64,
	CH_128
};

/** Channel distribution setting for OusterOS Lidars. */
UENUM(BlueprintType)
enum class EAGX_OusterOSBeamSpacing : uint8
{
	Uniform,
	AboveHorizon,
	BelowHorizon
};

/**
* Mode setting for OusterOS Lidars.
* The first number represents Horizontal Resolution.
* The second number represents frequency.
*/
UENUM(BlueprintType)
enum class EAGX_OusterOSMode : uint8
{
	Mode_512x10,
	Mode_512x20,
	Mode_1024x10,
	Mode_1024x20,
	Mode_2048x10
};

/** Ray angle noise axis setting. */
UENUM(BlueprintType)
enum class EAGX_LidarRayAngleDistortionAxis : uint8
{
	AxisX,
	AxisY,
	AxisZ
};
