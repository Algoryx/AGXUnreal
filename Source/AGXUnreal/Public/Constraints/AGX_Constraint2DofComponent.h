// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_Constraint2DOFFreeDOF.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Constraints/Controllers/AGX_ElectricMotorController.h"
#include "Constraints/Controllers/AGX_FrictionController.h"
#include "Constraints/Controllers/AGX_LockController.h"
#include "Constraints/Controllers/AGX_RangeController.h"
#include "Constraints/Controllers/AGX_ScrewController.h"
#include "Constraints/Controllers/AGX_TargetSpeedController.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Containers/StaticArray.h"

#include "AGX_Constraint2DofComponent.generated.h"

/**
 *
 */
UCLASS(
	ClassGroup = "AGX_Constraint", Category = "AGX", Abstract, Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_Constraint2DofComponent : public UAGX_ConstraintComponent
{
	GENERATED_BODY()

public:
	/** Electric motor controller for first secondary constraint (on one of the 2 free DOFs,
	 * usually). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraints")
	FAGX_ConstraintElectricMotorController ElectricMotorController1;

	/** Electric motor controller for second secondary constraint (on one of the 2 free DOFs,
	 * usually). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraints")
	FAGX_ConstraintElectricMotorController ElectricMotorController2;

	/** Friction controller for first secondary constraint (on one of the 2 free DOFs, usually). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraints")
	FAGX_ConstraintFrictionController FrictionController1;

	/** Friction controller for second secondary constraint (on one of the 2 free DOFs, usually). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraints")
	FAGX_ConstraintFrictionController FrictionController2;

	/** Lock controller for first secondary constraint (on one of the 2 free DOFs, usually). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraints")
	FAGX_ConstraintLockController LockController1;

	/** Lock controller for second secondary constraint (on one of the 2 free DOFs, usually). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraints")
	FAGX_ConstraintLockController LockController2;

	/** Range controller for first secondary constraint (on one of the 2 free DOFs, usually). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraints")
	FAGX_ConstraintRangeController RangeController1;

	/** Range controller for second secondary constraint (on one of the 2 free DOFs, usually). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraints")
	FAGX_ConstraintRangeController RangeController2;

	/** Target speed controller for first secondary constraint (on one of the 2 free DOFs, usually).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraints")
	FAGX_ConstraintTargetSpeedController TargetSpeedController1;

	/** Target speed controller for second secondary constraint (on one of the 2 free DOFs,
	 * usually). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraints")
	FAGX_ConstraintTargetSpeedController TargetSpeedController2;

	/** Screw controller that puts a relationship between the two free DOFs. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraints")
	FAGX_ConstraintScrewController ScrewController;

	/**
	 * Get the current angle or position of the given free degree of freedom (DOF). If the given DOF
	 * is translational then the position is in [cm], if this is a rotational constraint then the
	 * angle is in [deg].
	 * @return The angle [deg] or position [cm] of the given free degree of freedom.
	 */
	double GetAngle(EAGX_Constraint2DOFFreeDOF Dof) const;

	UAGX_Constraint2DofComponent();

	UAGX_Constraint2DofComponent(
		const TArray<EDofFlag>& LockedDofsOrdered, bool bIsSecondaryConstraint1Rotational,
		bool bIsSecondaryConstraint2Rotational);

	virtual ~UAGX_Constraint2DofComponent() override;

	virtual void UpdateNativeProperties() override;

	virtual void CopyFrom(const FConstraintBarrier& Barrier, FAGX_ImportContext* Context) override;

	TStaticArray<FAGX_ConstraintController*, 11> GetAllControllers();

	// ~Begin IAGX_NativeOwner interface.
	virtual void SetNativeAddress(uint64 NativeAddress) override;
	// ~End IAGX_NativeOwner interface.

	// ~ Begin UObject interface.
#if WITH_EDITOR
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& Event) override;
#endif
	virtual void Serialize(FArchive& Archive) override;
	// ~ End UObject interface.

protected:
	/**
	 * Call AllocateNative and then bind the constraint controllers to their native representations
	 * within the allocated native constraint.
	 */
	virtual void CreateNativeImpl() override final;

	/**
	 * Allocate the native constraint, of the appropriate type for the current subclass of
	 * UAGX_Constraint1DofComponent, and assign the new typed ConstraintBarrier to the inherited
	 * NativeBarrier member variable.
	 */
	virtual void AllocateNative() PURE_VIRTUAL(UAGX_Constraint2DofComponent::AllocateNativ, );
};
