// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

#include "AGX_TrackEnums.generated.h"

// Unreal Header Tool does not support line breaks in UMETA tags.
// clang-format off
/**
 * The different types of wheels supported by AGX Dynamics.
 *
 * The values of these must match the corresponding enum in AGX Dynamics.
 */
UENUM(BlueprintType)
enum class EAGX_TrackWheelModel : uint8
{
	Sprocket UMETA(DisplayName = "Sprocket", ToolTip = "Geared driving wheel. Will merge track nodes to itself."),
	Idler UMETA(DisplayName = "Idler", ToolTip = "Geared non-powered wheel. Will merge track nodes to itself."),
	Roller UMETA(DisplayName = "Roller", ToolTip = "Track return or road wheel.")
};
// clang-format on

/**
 * Contact reduction of merged nodes in contact with other objects such as ground.
 */
UENUM(BlueprintType)
enum class EAGX_MergedTrackNodeContactReduction : uint8
{
	/** Contact reduction disabled. */
	None,

	/** Contact reduction enabled with bin resolution = 3. */
	Minimal,

	/** Contact reduction enabled with bin resolution = 2. */
	Moderate,

	/** Contact reduction enabled with bin resolution = 1. */
	Aggressive
};
