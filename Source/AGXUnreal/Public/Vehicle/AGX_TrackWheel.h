// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyReference.h"
#include "AGX_SceneComponentReference.h"
#include "Vehicle/AGX_TrackEnums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "AGX_TrackWheel.generated.h"

/**
 * A track wheel referenced by the Track Component. Specifies the wheel rigid body,
 * the wheel transform, and interaction characteristics against the track nodes.
 *
 * Note that this object does not create any geometries, bodies, or joints on its own,
 * it merely acts a data connection between the already existing actual wheel Rigid Body
 * and the Track Component. In other words, it turns a body into a track wheel.
 * In particular, it will affect the routing of the track.
 *
 * This object is only used at BeginPlay by Track Component.
 */
USTRUCT(BlueprintType, Category = "AGX Dynamics")
struct AGXUNREAL_API FAGX_TrackWheel
{
	GENERATED_BODY()

public:
	/**
	 *  The Rigid Body to which this wheel belongs.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Track Wheel")
	FAGX_RigidBodyReference RigidBody;

	/**
	 * The optional Frame Defining Component makes it possible to use the transform of any Actor
	 * or Component to define the frame of the Track Wheel. If set, RelativeLocation and
	 * RelativeRotation are relative to the Frame Defining Component instead of to the Rigid Body
	 * Component.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Track Wheel",
		Meta = (EditCondition = "bUseFrameDefiningComponent"))
	FAGX_SceneComponentReference FrameDefiningComponent;

	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Track Wheel",
		Meta = (InlineEditConditionToggle))
	bool bUseFrameDefiningComponent {false};

	/**
	 * Wheel location relative to the Rigid Body,
	 * or to the FrameDefiningComponent if bUseFrameDefiningComponent is true [cm].
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Track Wheel")
	FVector RelativeLocation {FVector::ZeroVector};

	/**
	 * Wheel rotation relative to the Rigid Body,
	 * or to the FrameDefiningComponent if bUseFrameDefiningComponent is true [deg].
	 *
	 * The Y-Axis becomes the rotation-axis, and the Z-axis becomes the up-axis.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Track Wheel")
	FRotator RelativeRotation {FRotator::ZeroRotator};

	/**
	 * Radius of the wheel [cm].
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Track Wheel")
	float Radius = 50.0f;

	/**
	 * The wheel type. Sprocket and Idler wheel types will merge track nodes to them,
	 * simulating geared, non-slip, type of properties.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Track Wheel")
	EAGX_TrackWheelModel Model = EAGX_TrackWheelModel::Roller;

	/**
	 * Wheels with this property will split segments, i.e., intermediate nodes that are merged.
	 * Use this property on ROLLER models where the track changes angle.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Track Wheel")
	bool bSplitSegments {false};

	/**
	 * If enabled - when a node is merged to the wheel, move the node into the plane defined by
	 * the wheel center position and rotation axis. This will prevent the tracks from sliding of
	 * its path but all Sprocket and Idler wheels must be aligned.
	 *
	 * Only relevant for Idler or Sprocket wheels.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Track Wheel")
	bool bMoveNodesToRotationPlane {false};

	/**
	 * Similar to 'Move Nodes To Rotation Plane' but this property will also make sure
	 * the node is moved to the surface of the wheel.
	 *
	 * Only relevant for Idler or Sprocket wheels.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Track Wheel")
	bool bMoveNodesToWheel {false};

	// \note Not exposing the MERGE_NODES flag here because it seems to automatically be set
	// internally by AgxDynamics dependent on Model, so it is redundant and therefore unnecessary to
	// expose. User can just set track wheel model to sprocket or idler to get the merge-to-wheel
	// behaviour.

	static EAGX_TrackWheelModel ToModel(uint8 Model);

public:
	bool GetTransformRelativeToBody(FVector& RelPosition, FQuat& RelRotation) const;
};
