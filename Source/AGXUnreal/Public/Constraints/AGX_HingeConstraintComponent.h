// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_Constraint1DofComponent.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_HingeConstraintComponent.generated.h"

class FHingeBarrier;

/**
 * Locks all degrees of freedom except for rotation around the Z-axis.
 */
UCLASS(
	ClassGroup = "AGX_Constraint", Category = "AGX", Blueprintable,
	Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_HingeConstraintComponent : public UAGX_Constraint1DofComponent
{
	GENERATED_BODY()

public:
	using FBarrierType = FHingeBarrier;

public:
	UAGX_HingeConstraintComponent();
	virtual ~UAGX_HingeConstraintComponent() override;

	FHingeBarrier* GetNativeHinge();
	const FHingeBarrier* GetNativeHinge() const;

protected:
	virtual void AllocateNative() override;
};
