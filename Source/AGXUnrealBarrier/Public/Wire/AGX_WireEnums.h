// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

#include "AGX_WireEnums.generated.h"

/**
 * Types of wire nodes we support in AGX Dynamics for Unreal.
 *
 * Suitable for array indexing.
 */
UENUM(BlueprintType)
enum class EWireNodeType : uint8
{
	Free,
	Eye,
	BodyFixed,
	NUM_USER_CREATABLE UMETA(HIDDEN),
	Connecting UMETA(HIDDEN),
	Stop UMETA(HIDDEN), // Internal node created by Wire Winch.
	Contact UMETA(HIDDEN), // Internal node created by dynamics contacts.
	ShapeContact UMETA(HIDDEN), // Internal node created by dynamics contacts.
	Other UMETA(HIDDEN), // Any other node type that we don't allow a user to create directly.
	NUM_NODE_TYPES UMETA(HIDDEN) // Include NUM_USER_CREATABLE so that all can be used as an index.
};

/**
 * All wire node types in AGX Dynamics for Unreal.
 *
 * Bitfield, not suitable for array indexing
 */
UENUM()
enum class EWireNodeNativeType : uint8
{
	// These much match the values of agxWire::Node::Type in agxWire/Node.h.
	NOT_DEFINED = 0,
	Eye = (1 << 0),
	Missing = (1 << 1),
	Connecting = (1 << 2),
	Free = (1 << 3),
	Contact = (1 << 4),
	BodyFixed = (1 << 5),
	Stop = (1 << 6),
	ShapeContact = (1 << 7)
};

/**
 * The type of Component that this Wire Winch is part of, or which type of Component this Wire Winch
 * points to if we are currently modifying a Wire Component.
 */
UENUM()
enum class EWireWinchOwnerType : uint8
{
	// No which.
	None,

	// When used as an Outer designator, the Wire Winch is on one of the two ends of a Wire
	// Component. We are not editing an actual Wire Winch, but a reference to one.
	//
	// When used as a Target designator, a new Wire Winch instance should be created for the Wire
	// Component.
	Wire,

	// When used as an Outer designator, the Wire Winch is on a Wire Winch Component.
	//
	// When used as a Target designator, the Wire should have a pointer to the Wire Winch inside a
	// Wire Winch Component.
	WireWinch,

	// When used as an Outer designator, the Wire Winch is part of something that is neither a Wire
	// Component nor a Wire Winch Component. Much of the Wire Winch Details functionality is
	// disabled in this case.
	//
	// When used as a Target designator, the Wire relinquish all responsibility of the the Wire
	// Winch settings setup and will use whatever UAGX_WireComponent::BorrowedBeginWinch points to.
	Other
};

/**
 * Used to select which side of a wire is being operator on. The Begin side is the side represented
 * by the wire's first route node, while the end side is the on at the node last in the wires route
 * nodes list.
 */
UENUM(BlueprintType)
enum class EWireSide : uint8
{
	None,
	Begin,
	End
};

/*
 * Used to select either the location Hit Proxy or the rotation Hit Proxy when manipulating a Wire
 * Winch using the Component Visualizer.
 */
UENUM()
enum class EWinchSide : uint8
{
	None,
	Location,
	Rotation
};
