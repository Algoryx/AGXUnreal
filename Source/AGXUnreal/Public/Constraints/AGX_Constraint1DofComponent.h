// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_ConstraintComponent.h"
#include "Constraints/Controllers/AGX_ElectricMotorController.h"
#include "Constraints/Controllers/AGX_FrictionController.h"
#include "Constraints/Controllers/AGX_LockController.h"
#include "Constraints/Controllers/AGX_RangeController.h"
#include "Constraints/Controllers/AGX_TargetSpeedController.h"

// Unreal Engine includes.
#include "Containers/StaticArray.h"
#include "CoreMinimal.h"

#include "AGX_Constraint1DofComponent.generated.h"

/**
 *
 */
UCLASS(
	ClassGroup = "AGX_Constraint", Category = "AGX", Abstract, Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_Constraint1DofComponent : public UAGX_ConstraintComponent
{
	GENERATED_BODY()

public:
	/** Electric motor controller for the secondary constraint (on the free DOF, usually). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraint")
	FAGX_ConstraintElectricMotorController ElectricMotorController;

	/** Friction controller for the secondary constraint (on the free DOF, usually). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraint")
	FAGX_ConstraintFrictionController FrictionController;

	/** Lock controller for the secondary constraint (on the free DOF, usually). */
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraint",
		Meta = (EditCondition = "bIsLockControllerEditable"))
	FAGX_ConstraintLockController LockController;

	/** Range controller for the secondary constraint (on the free DOF, usually). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraint")
	FAGX_ConstraintRangeController RangeController;

	/** Target speed controller for the secondary constraint (on the free DOF, usually). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraint")
	FAGX_ConstraintTargetSpeedController TargetSpeedController;

	/**
	 * Get the current angle or position of the free degree of freedom. If this is a translational
	 * constraint then the position is in [cm], if this is a rotational constraint then the angle
	 * is in [deg].
	 * @return The angle [deg] or position [cm] of the free degree of freedom.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Constraint")
	double GetAngle() const;

	/**
	 * Get the current speed in the free degree of freedom. If this is a translational constraint
	 * then the speed is in [cm/s], if it is a rotational constraint then the speed is in [deg/s].
	 * @return The speed, either [cm/s] or [rad/s].
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Constraint")
	double GetSpeed() const;

	UAGX_Constraint1DofComponent() = default;
	UAGX_Constraint1DofComponent(const TArray<EDofFlag>& LockedDofsOrdered, bool bIsLockControllerEditable = true);

	virtual ~UAGX_Constraint1DofComponent() override;

	virtual void UpdateNativeProperties() override;

	virtual void CopyFrom(const FConstraintBarrier& Barrier, FAGX_ImportContext* Context) override;

	TStaticArray<FAGX_ConstraintController*, 5> GetAllControllers();

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
	virtual void CreateNativeImpl() override;

	/**
	 * Allocate the native constraint, of the appropriate type for the current subclass of
	 * UAGX_Constraint1DofComponent, and assign the new typed ConstraintBarrier to the inherited
	 * NativeBarrier member variable.
	 */
	virtual void AllocateNative() PURE_VIRTUAL(UAGX_Constraint1DofComponent::AllocateNative, );

public: // Deprecated functions.
	/**
	 * Get the current angle or position of the free degree of freedom. If this is a translational
	 * constraint then the position is in [cm], if this is a rotational constraint then the angle
	 * is in [deg].
	 * @return The angle [deg] or position [cm] of the free degree of freedom.
	 */
	UFUNCTION(
		BlueprintCallable, BlueprintPure, Category = "AGX Constraint",
		Meta = (DeprecatedFunction, DeprecationMessage = "Use GetAngle instead of GetAngle_BP"))
	float GetAngle_BP() const;

	/**
	 * Get the current speed in the free degree of freedom. If this is a translational constraint
	 * then the speed is in [cm/s], if it is a rotational constraint then the speed is in [deg/s].
	 * @return The speed, either [cm/s] or [rad/s].
	 */
	UFUNCTION(
		BlueprintCallable, BlueprintPure, Category = "AGX Constraint",
		Meta = (DeprecatedFunction, DeprecationMessage = "Use GetSpeed instead of GetSpeeds_BP"))
	float GetSpeed_BP() const;

private:
	// Some constraints, such as Distance, cannot use the lock constraint because it is reserved for
	// internal use within the native constraint.
	UPROPERTY(Transient, Category = "AGX Secondary Constraint", VisibleDefaultsOnly)
	bool bIsLockControllerEditable;
};
