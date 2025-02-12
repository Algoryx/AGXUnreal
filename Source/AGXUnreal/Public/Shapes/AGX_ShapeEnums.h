// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

/**
 * Enum that specify how much contact data the should be generated for a geometry that has been
 * marked as a sensor.
 */
UENUM(BlueprintType)
enum class EAGX_ShapeSensorType : uint8
{
	/**
	 * This shape will generate contact point information. Note that this is alternative is more
	 * computationally expensive than the Boolean Sensor setting.
	 */
	ContactsSensor,

	/**
	 * This shape will not generate contact point information, but will detect if this shape is in
	 * contact with other shapes. This alternative may be used to increase performance where contact
	 * point data is not needed.
	 */
	BooleanSensor
};

/**
 * Specifies from where to get the triangle data.
 */
UENUM(BlueprintType)
enum EAGX_StaticMeshSourceLocation
{
	/** Static Mesh from the first child component that is a Static Mesh Component. */
	TSL_CHILD_STATIC_MESH_COMPONENT UMETA(DisplayName = "Child Component"),

	/** Static Mesh from the first ancestor that is a Static Mesh Component. */
	TSL_PARENT_STATIC_MESH_COMPONENT UMETA(DisplayName = "Parent Component"),

	/** Directly from explicitly chosen Static Mesh Asset. */
	TSL_STATIC_MESH_ASSET UMETA(DisplayName = "Asset")
};
