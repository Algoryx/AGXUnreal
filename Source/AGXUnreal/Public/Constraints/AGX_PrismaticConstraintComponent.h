// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_Constraint1DofComponent.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_PrismaticConstraintComponent.generated.h"

class FPrismaticBarrier;

/**
 * Locks all degrees of freedom except for translation along the Z-axis.
 */
UCLASS(
	ClassGroup = "AGX_Constraint", Category = "AGX", Blueprintable,
	Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_PrismaticConstraintComponent : public UAGX_Constraint1DofComponent
{
	GENERATED_BODY()

public:
	using FBarrierType = FPrismaticBarrier;

public:
	UAGX_PrismaticConstraintComponent();
	virtual ~UAGX_PrismaticConstraintComponent();

	FPrismaticBarrier* GetNativePrismatic();
	const FPrismaticBarrier* GetNativePrismatic() const;

protected:
	virtual void AllocateNative() override;
};
