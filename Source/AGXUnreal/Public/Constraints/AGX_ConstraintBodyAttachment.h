// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyReference.h"
#include "AGX_SceneComponentReference.h"
#include "Constraints/AGX_ConstraintEnums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"

#include "AGX_ConstraintBodyAttachment.generated.h"

class UAGX_ConstraintComponent;
class UAGX_RigidBodyComponent;
class AAGX_ConstraintFrameActor;
struct FRigidBodyBarrier;

/**
 * Defines the Rigid Body to be bound by a Constraint and an attachment frame that is
 * defined by the transform of either the Constraint itself, the Rigid Body or some other Actor
 * or Component plus an optional offset given by the Local Frame Location and Rotation.
 *
 * Whether the constraint itself, the Rigid Body or some other Actor or Component should be used
 * to define the attachment frame can be selected by changing the Frame Defining Source accordingly.
 */
USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_ConstraintBodyAttachment
{
	GENERATED_BODY()

	FAGX_ConstraintBodyAttachment() = default;
	FAGX_ConstraintBodyAttachment(USceneComponent* InOuter);
	FAGX_ConstraintBodyAttachment& operator=(const FAGX_ConstraintBodyAttachment& Other);

	/// \todo Cannot assume a single body per actor. Should we change the UPROPERTY
	/// to be a UAGX_RigidBodyComponent instead, or should we keep the Actor
	/// reference and also keep some kind of component identifier? Should we use
	/// FComponentRef here?

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rigid Body")
	FAGX_RigidBodyReference RigidBody;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Transformation")
	TEnumAsByte<enum EAGX_FrameDefiningSource> FrameDefiningSource =
		EAGX_FrameDefiningSource::Constraint;

	/**
	 * The Frame Defining Component makes it possible to use the transform of any Actor or Component
	 * to define the attachment frame of the Constrained Rigid Body.
	 *
	 * Note that the two rigid bodies in a  constraint can use the same Frame Defining Component, or
	 * different, or one can have one and the other not.
	 * AGX Dynamics for Unreal provides Constraint Frame Component which is intended for this
	 * purpose. It provides constraint listing and visualization making it possible to see which
	 * constraints are using that Constraint Frame Component.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "Frame Transformation",
		Meta = (EditCondition = "FrameDefiningSource == EAGX_FrameDefiningSource::Other"))
	FAGX_SceneComponentReference FrameDefiningComponent;

	/** Frame location relative to either the Constraint, the Rigid Body Actor or from the Frame
	 * Defining Actor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Transformation")
	FVector LocalFrameLocation {FVector::ZeroVector};

	/** Frame rotation relative to to either the Constraint, the Rigid Body Actor or from the Frame
	 * Defining Actor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Transformation")
	FRotator LocalFrameRotation {FRotator::ZeroRotator};

	UAGX_RigidBodyComponent* GetRigidBody() const;

	/**
	 * Calculates and returns the frame location given in the RigidBody's frame.
	 * Returns zero vector if RigidBody has not been set.
	 */
	FVector GetLocalFrameLocationFromBody() const;

	/**
	 * Calculates and returns the frame rotation given in the RigidBody's frame.
	 * Returns identity quaternion of RigidBody has not been set.
	 */
	FQuat GetLocalFrameRotationFromBody() const;

	/**
	 * Calculates and returns the frame location in world space.
	 */
	FVector GetGlobalFrameLocation() const;

	/**
	 * Calculates and returns the frame rotation in world space.
	 */
	FQuat GetGlobalFrameRotation() const;

	FMatrix GetGlobalFrameMatrix() const;

	/**
	 * Get the RigidBodyBarrier owned by the RigidBodyComponent this Attachment points to.
	 *
	 * Will return nullptr if this Attachment doesn't have a RigidBodyComponent, or if the body
	 * doesn't yet have a native AGX Dynamics RigidBody representation.
	 *
	 * @see GetOrCreateRigidBodyBarrier
	 * @return The Barrier object for the RigidBody that this Attachment references, or nullptr.
	 */
	FRigidBodyBarrier* GetRigidBodyBarrier();

	/**
	 * Get the RigidBodyBarrier owned by the RigidBodyComponent this Attachment points to.
	 *
	 * Will return nullptr if this Attachment doesn't have a RigidBodyComponent.
	 *
	 * A native AGX Dynamics RigidBody representation will be created for the RigidBodyComponent if
	 * one hasn't already been created.
	 *
	 * @see GetRigidBodyBarrier
	 * @return The Barrier object for the RigidBody that this Attachment references.
	 */
	FRigidBodyBarrier* GetOrCreateRigidBodyBarrier();

	/**
	 * The USceneComponent, often a subclass of UAGX_ConstraintComponent, that this Attachment is
	 * part of. Used when FrameDefiningSource is set to Constraint, which will make this Attachment
	 * relative to this Component's world location.
	 *
	 * This is organizational data and not part of the salient value of the type. It should
	 * therefore not be modified by Unreal Engine in any way. This means that Object Initialization
	 * shouldn't overwrite it with bytes from the Class Default Object and it should not be
	 * serialized. Whatever object owns this Attachment should have full control over this pointer.
	 *
	 * This used to be a UPROPERTY in order to incentivize Unreal Engine to keep this pointer
	 * up-to-date as objects are being duplicated during world setup. This worked in many cases,
	 * but sometimes the update ofter a duplication would fail with incorrect constraint frames
	 * as a result. Instead of having Unreal Engine update this pointer for us we now try really
	 * hard to not have it change, ever, by not marking it with UPROPERTY and implementing operator=
	 * in a way that doesn't copy it. We need operator= because during creation Unreal Engine
	 * assigns from the Class Default Object to the new instance with operator=. We may also need to
	 * implement the copy constructor, but that need hasn't been demonstrated yet.
	 *
	 * We would like this to be const, so that it can be set the constructor and then never changed
	 * again, but Unreal Engine need to be able to create instances of this class using the default
	 * constructor and we may want to update the Outer on such default created instances.
	 */
	USceneComponent* Outer;

#if WITH_EDITOR
	/**
	 * Should be invoked whenever FrameDefiningComponent changes, to trigger the
	 * removal of the constraint from the previous FrameDefiningComponent's list of
	 * constraint usages, and adding to the new one's (if they are
	 * AAGX_ConstraintFrameActor actor types).
	 */
	void OnFrameDefiningComponentChanged(UAGX_ConstraintComponent* Parent);

	void UnregisterFromConstraintFrameComponent(UAGX_ConstraintComponent* Parent);
#endif

private:
	/**
	 * Used only to be able to call some cleanup functions on previous Frame Defining Actor
	 * whenever Frame Defining Actor is set to another actor.
	 */
	UPROPERTY(Transient)
	mutable AActor* RecentFrameDefiningActor = nullptr;
	USceneComponent* PreviousFrameDefiningComponent;

	// Returns the currently active FrameDefiningComponent given the FrameDefiningSource selected.
	USceneComponent* GetFinalFrameDefiningComponent() const;
};
