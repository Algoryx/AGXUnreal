// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

#include "AGX_TrackEnums.generated.h"

/**
 * The runtime implementation used by AGX Dynamics Track.
 */
UENUM(BlueprintType)
enum class EAGX_TrackImplementation : uint8
{
	/**
	 * The default AGX Track Model does not model individual track shoes explicitly. Instead, it
   * uses a compact set of bodies and constraints, where the degrees of freedom are determined by
   * the number of wheels rather than the number of track nodes.

   * Between each pair of wheels, a sensor box body is introduced and constrained to the chassis
   * using a prismatic constraint. Track-ground interaction is represented through the combined
   * behavior of the wheels and sensor boxes, which together define the track topology and tension
   * forces. The track nodes are animated based on wheel geometry and rotational motion, enabling
   * realistic visual rendering.

   * This approach provides a stable and efficient simulation while capturing the essential dynamics
   * of tracked vehicles.
	 */
	Default,

	/**
	 * Use the full degrees-of-freedom AGX Dynamics track model.
	 * The Full-DOF Track Model is based on a lumped-element approach with multiple
	 * degrees-of-freedom. Each track node is represented by a rigid body connected through hinges,
	 * accurately capturing the physical constraints between components and their interactions with
	 * terrain and other bodies. This model delivers high physical realism and is well suited for
	 * applications where detailed track behavior is important.
	 */
	FullDOF
};

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
