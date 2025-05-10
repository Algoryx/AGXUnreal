// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_Constraint1DofComponent.h"
#include "Constraints/AGX_ConstraintEnumsCommon.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_SingleControllerConstraint1DOFComponent.generated.h"

class FSingleControllerConstraint1DOFBarrier;

struct FAGX_ImportContext;

/**
 * This constraint is meant as a container for a single 1DOF basic controller constraint without
 * creating any elementary constraints associated with the archetypal ordinary constraints.
 */
UCLASS(
	ClassGroup = "AGX_Constraint", Category = "AGX", Blueprintable,
	Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_SingleControllerConstraint1DofComponent
	: public UAGX_Constraint1DofComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraint")
	EAGX_ConstraintControllerType ControllerType {EAGX_ConstraintControllerType::Invalid};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraint")
	EAGX_ConstraintAngleControllerType ControllerAngleType {
		EAGX_ConstraintAngleControllerType::RotationalAngle};

	using FBarrierType = FSingleControllerConstraint1DOFBarrier;

public:
	UAGX_SingleControllerConstraint1DofComponent();
	virtual ~UAGX_SingleControllerConstraint1DofComponent() override;

	FSingleControllerConstraint1DOFBarrier* GetNativeFingleControllerConstraint1DOF();
	const FSingleControllerConstraint1DOFBarrier* GetNativeSingleControllerConstraint1DOF() const;

	virtual void CopyFrom(const FConstraintBarrier& Barrier, FAGX_ImportContext* Context) override;

	virtual bool GetValid() const override;

protected:
	virtual void CreateNativeImpl() override;
};
