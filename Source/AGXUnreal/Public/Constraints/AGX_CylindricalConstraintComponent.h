// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_Constraint2DofComponent.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_CylindricalConstraintComponent.generated.h"

class FCylindricalJointBarrier;

/**
 * Locks all degrees of freedom except for translation and rotation along/around the Z-axis.
 */
UCLASS(ClassGroup = "AGX_Constraint", Category = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_CylindricalConstraintComponent : public UAGX_Constraint2DofComponent
{
	GENERATED_BODY()

public:
	using FBarrierType = FCylindricalJointBarrier;

public:
	UAGX_CylindricalConstraintComponent();
	virtual ~UAGX_CylindricalConstraintComponent() override;

	FCylindricalJointBarrier* GetNativeCylindrical();
	const FCylindricalJointBarrier* GetNativeCylindrical() const;

protected:
	virtual void AllocateNative() override;
};
