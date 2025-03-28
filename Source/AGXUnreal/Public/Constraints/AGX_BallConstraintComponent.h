// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_ConstraintComponent.h"
#include "Constraints/Controllers/AGX_TwistRangeController.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_BallConstraintComponent.generated.h"

class FBallJointBarrier;

/**
 * Locks all translational degrees of freedom, but rotation is free.
 */
UCLASS(ClassGroup = "AGX_Constraint", Category = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_BallConstraintComponent : public UAGX_ConstraintComponent
{
	GENERATED_BODY()

public: // Type aliases.
	using FBarrierType = FBallJointBarrier;

public: // Special member functions.
	UAGX_BallConstraintComponent();
	virtual ~UAGX_BallConstraintComponent() override;

public: // Properties.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraints")
	FAGX_TwistRangeController TwistRangeController;

public: // Function overrides.
	//~ Begin AGX Constraint Component interface.
	virtual void UpdateNativeProperties() override;
	//~ End AGX Constraint Component interface.

	//~ Begin IAGX_NativeOwner interface.
	virtual void SetNativeAddress(uint64 NativeAddress) override;
	//~ End IAGX_NativeOwner interface.

	//~ Begin UObject interface.
#if WITH_EDITOR
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& Event) override;
#endif
	//~ End UObject interface.

	virtual void CopyFrom(const FConstraintBarrier& Barrier, FAGX_ImportContext* Context) override;

public: // Native management.
	FBallJointBarrier* GetNativeBallJoint();
	const FBallJointBarrier* GetNativeBallJoint() const;

protected:
	virtual void CreateNativeImpl() override;
};
