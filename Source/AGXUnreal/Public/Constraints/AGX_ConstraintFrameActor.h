// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "AGX_ConstraintFrameActor.generated.h"

class UAGX_ConstraintComponent;
class UAGX_ConstraintFrameComponent;

/**
 * Actor helper that can be used to define the location and rotation of
 * a Constraint Attachment Frame in a controlled way.
 *
 * Reference this object from an AGX Constraint.
 *
 * It can be referenced by both Rigid Bodies of the same AGX Constraint,
 * which makes it easy to position the constraint frame anywhere in the world.
 *
 */
UCLASS(
	ClassGroup = "AGX", Category = "AGX", Meta = (BlueprintSpawnableComponent),
	hidecategories = (Cooking, Collision, Input, LOD, Rendering, Replication))
class AGXUNREAL_API AAGX_ConstraintFrameActor : public AActor
{
	GENERATED_BODY()

public:
	/** Sets default values for this actor's properties. */
	AAGX_ConstraintFrameActor();

	/** Indicates whether this actor should participate in level bounds calculations. */
	bool IsLevelBoundsRelevant() const override
	{
		return false;
	}

private:
	UPROPERTY()
	UAGX_ConstraintFrameComponent* ConstraintFrameComponent;

public:
	void AddConstraintUsage(UAGX_ConstraintComponent* Constraint);

	void RemoveConstraintUsage(UAGX_ConstraintComponent* Constraint);

	const TArray<UAGX_ConstraintComponent*>& GetConstraintUsage() const;
};
