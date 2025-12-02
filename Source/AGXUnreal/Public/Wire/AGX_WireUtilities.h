// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Wire/AGX_WireEnums.h"

// Unreal Engine includes.
#include "Math/Vector.h"
#include "Math/Rotator.h"

struct FAGX_WireWinch;
class UAGX_WireComponent;
class UAGX_WireWinchComponent;

struct FAGX_WireWinchPose
{
	const FTransform& LocalToWorld; // Will reference into object that owns the winch.
	const FVector LocalLocation;
	const FRotator LocalRotation;
};

class AGXUNREAL_API FAGX_WireUtilities
{
public:
	/**
	 * Get the pose of the active Wire Winch on the given side of the given wire.
	 *
	 * The pose is the local location and rotation of the winch along with the transformation that
	 * brings the local pose to the world coordinate system. What transformation is used depend on
	 * what type of Component, Wire or Wire Winch, owns the Wire Winch, if the winch is attached to
	 * a body and if an AGX Dynamics instance of the Wire Winch has been created.
	 */
	static FAGX_WireWinchPose GetWireWinchPose(const UAGX_WireComponent& Wire, EWireSide Side);

	/**
	 * Get the pose of the Wire Winch.
	 *
	 * The pose is the local location and rotation of the winch along with the transformation that
	 * brings the local pose to the world coordinate system. What transformation is used depend on
	 * if the simulation has been started, and if the Wire Winch is attached to a body or not.
	 */
	static FAGX_WireWinchPose GetWireWinchPose(const UAGX_WireWinchComponent& WireWinch);

	/**
	 * @return The transformation that the Wire Winch's location and rotation is expressed in.
	 */
	static const FTransform& GetWinchLocalToWorld(const UAGX_WireComponent& Wire, EWireSide Side);

	/**
	 * @return The transformation that the Wire Winch's location and rotation is expressed in.
	 */
	static const FTransform& GetWinchLocalToWorld(const UAGX_WireWinchComponent& WireWinch);

	/**
	 * Compute the simulation time location and rotation on the given Wire owned Wire Winch,
	 * expressing them in the coordinate frame expected by AGX Dynamics.
	 *
	 * If the Wire Winch does not have a body then the world coordinate system is used.
	 * If the Wire Winch does have a body then the body's coordinate system is used.
	 */
	static void ComputeSimulationPlacement(const UAGX_WireComponent& Owner, FAGX_WireWinch& Winch);

	/**
	 * Compute the simulation time location and rotation on the given Wire Winch owned Wire Winch,
	 * expressing them in the coordinate frame expected by AGX Dynamics.
	 *
	 * If the Wire Winch does not have a body then the world coordinate system is used.
	 * If the Wire Winch does have a body then the body's coordinate system is used.
	 */
	static void ComputeSimulationPlacement(
		const UAGX_WireWinchComponent& Owner, FAGX_WireWinch& Winch);

	/**
	 * Compute the simulation time location and rotation of the given non-owned Wire Winch,
	 * expressing them in the coordinate frame expected by AGX Dynamics.
	 *
	 * Since the winch doesn't have an owner there is no context to transform the location and
	 * rotation in relation to. The transformation is therefore the identity function, i.e., the
	 * LocationSim and RotationSim member variables become copies of the Location and Rotation
	 * UProperties.
	 */
	static void ComputeSimulationPlacement(FAGX_WireWinch& Winch);

	// The size of Hit Proxies created by the various Wire-related Visualizers we have.
	// This may become a configurable setting in the future.
	static constexpr float NodeHandleSize = 10.0f;
};
