// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_ConstraintComponent.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_LockConstraintComponent.generated.h"

class FLockJointBarrier;

/**
 * Locks all degrees of freedom.
 */
UCLASS(ClassGroup = "AGX_Constraint", Category = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_LockConstraintComponent : public UAGX_ConstraintComponent
{
	GENERATED_BODY()

public:
	using FBarrierType = FLockJointBarrier;

public:
	UAGX_LockConstraintComponent();
	virtual ~UAGX_LockConstraintComponent();

	FLockJointBarrier* GetNativeLock();
	const FLockJointBarrier* GetNativeLock() const;

protected:
	virtual void CreateNativeImpl() override;
};
