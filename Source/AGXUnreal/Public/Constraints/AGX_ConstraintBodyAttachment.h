#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "AGX_ConstraintBodyAttachment.generated.h"


class AAGX_Constraint;
class AAGX_ConstraintFrameActor;
class FRigidBodyBarrier;


/**
 * Defines the Rigid Body to be bound by a Constraint and its Local Frame Location
 * and Rotation.
 *
 * The actual usage of the Local Frame Location and Rotation varies dependening on
 * constraint type, but it can generally be seen as the local points (on the rigid bodies)
 * that should in some way be glewed together by the constraint.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ConstraintBodyAttachment
{
	GENERATED_USTRUCT_BODY()

	/**
	 * The Actor containing the Rigid Body Component to be bound by the constraint.
	 */
	UPROPERTY(EditAnywhere, Category = "Rigid Body")
	AActor* RigidBodyActor;

	/**
	 * Optional. Use this to define the Local Frame Location and Rotation relative to
	 * an actor other than the Rigid Body Actor (or to use the other Actor's transform
	 * directly by setting Local Frame Location and Rotation to zero). It is recommended
	 * to use the dedicated AGX Constraint Frame Actor, but any other actor can also be used.
	 *
	 * This is used for convenience only. The actual local frame transform used by
	 * the simulation will nevertheless be calculated and stored relative to the
	 * rigid body when the simulation is starting.
	 *
	 * Note that both rigid bodies can use the same frame defining actor, or one rigid body
	 * can use the other rigid body as frame defining actor, etc.
	 */
	UPROPERTY(EditAnywhere, Category = "Frame Transformation")
	AActor* FrameDefiningActor;

	/** Frame location relative to Rigid Body Actor, or from Frame Defining Actor if set. */
	UPROPERTY(EditAnywhere, Category = "Frame Transformation")
	FVector LocalFrameLocation;

	/** Frame rotation relative to Rigid Body Actor, or from Frame Defining Actor if set. */
	UPROPERTY(EditAnywhere, Category = "Frame Transformation")
	FRotator LocalFrameRotation;

	/**
	 * Calculates and returns the frame location relative to Rigid Body Actor
	 * (or in world space if not set). I.e. if Frame Defining Actor is set, the
	 * returned value is Local Frame Location transformed from Frame Defining Actor's
	 * to Rigid Body Actor's transform space, and if not set the returned value is
	 * just Local Frame Location.
	 */
	FVector GetLocalFrameLocation() const;

	/**
	 * Calculates and returns the frame rotation relative to Rigid Body Actor
	 * (or in world space if not set). I.e. if Frame Defining Actor is set, the
	 * returned value is Local Frame Rotation transformed from Frame Defining Actor's
	 * to Rigid Body Actor's transform space, and if not set the returned value is
	 * just Local Frame Rotation.
	 */
	FQuat GetLocalFrameRotation() const;

	/**
	 * Calculates and returns the frame location in world space.
	 */
	FVector GetGlobalFrameLocation() const;

	/**
	 * Calculates and returns the frame rotation in world space.
	 */
	FQuat GetGlobalFrameRotation() const;

	FMatrix GetGlobalFrameMatrix() const;

	FRigidBodyBarrier* GetRigidBodyBarrier(bool CreateIfNeeded);

#if WITH_EDITOR
	/**
	 * Should be invoked whenever Frame Defining Actor changes, to trigger the removal
	 * of the constraint from the previous Frame Defining Actor's list of constraint usages,
	 * and adding to the new one's (if they are AAGX_ConstraintFrameActor actor types).
	 */
	void OnFrameDefiningActorChanged(AAGX_Constraint* Owner);

	void OnDestroy(AAGX_Constraint* Owner);

private:
	/**
	 * Used only to be able to call some cleanup functions on previous Frame Defining Actor
	 * whenever Frame Defining Actor is set to another actor.
	 */
	UPROPERTY(Transient)
	mutable AActor* RecentFrameDefiningActor;
#endif
};