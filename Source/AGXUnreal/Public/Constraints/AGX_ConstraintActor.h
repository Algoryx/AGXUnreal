// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "AGX_ConstraintActor.generated.h"

class UAGX_ConstraintComponent;

/**
 * Abstract base class for all AGX constraint types.
 *
 * Does not have a its own world space transform, but has references to the two
 * Rigid Body Actors to constrain to each other, and their attachment frames
 * (which defines how the rigid bodies should locally be jointed to one another).
 *
 * At least the first Rigid Body Actor must be chosen. If there is no second Rigid Body Actor,
 * then the first one will be constrained to the static World instead.
 *
 * Each non-abstract subclass must:
 *   1. Implement CreateNativeImpl (see method comment).
 *   2. In the constructor pass the constraint type specific array of locked DOFs to the
 *       overloaded AAGX_Constraint constructor. The array items and their indexes must exactly
 *       match the enum in the header of the native AGX constraint (without ALL_DOF and NUM_DOF).
 *
 */
UCLASS(
	ClassGroup = "AGX_Constraint", Category = "AGX", Abstract, NotBlueprintable,
	Meta = (BlueprintSpawnableComponent),
	hidecategories = (Cooking, Collision, Input, LOD, Rendering, Replication))
class AGXUNREAL_API AAGX_ConstraintActor : public AActor
{
	GENERATED_BODY()

public:
	/// \todo Can this constructor be removed?
	AAGX_ConstraintActor();
	virtual ~AAGX_ConstraintActor() override;

	UAGX_ConstraintComponent* GetConstraintComponent();
	const UAGX_ConstraintComponent* GetConstraintComponent() const;

protected:
	void SetConstraintComponent(UAGX_ConstraintComponent* InConstraintComponent);

public:
	/** Indicates whether this actor should participate in level bounds calculations. */
	bool IsLevelBoundsRelevant() const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Constraint Actor")
	UAGX_ConstraintComponent* ConstraintComponent;
};
