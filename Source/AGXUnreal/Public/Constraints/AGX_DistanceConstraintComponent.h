// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_Constraint1DofComponent.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_DistanceConstraintComponent.generated.h"

class FDistanceJointBarrier;

/**
 * Locks the initial relative distance between two bodies or a single
 * body and the world.
 * The Distance Constraint does not use an elementary constraint
 * which is why most of those properties are uneditable. Instead the
 * Distance Constraint is implemented using the secondary
 * constraints of which the Lock Controller is enabled by default.
 * This reflects the behavior of the AGX Dynamics' Distance Joint.
 */
UCLASS(
	ClassGroup = "AGX_Constraint", Category = "AGX", Blueprintable,
	Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_DistanceConstraintComponent : public UAGX_Constraint1DofComponent
{
	GENERATED_BODY()

public:
	using FBarrierType = FDistanceJointBarrier;

public:
	UAGX_DistanceConstraintComponent();
	virtual ~UAGX_DistanceConstraintComponent();

	FDistanceJointBarrier* GetNativeDistance();
	const FDistanceJointBarrier* GetNativeDistance() const;

protected:
	virtual void AllocateNative() override;
};
