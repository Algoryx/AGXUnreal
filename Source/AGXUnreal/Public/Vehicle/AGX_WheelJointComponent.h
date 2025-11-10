// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_ConstraintComponent.h"
#include "Constraints/AGX_ConstraintEnumsCommon.h"
#include "Constraints/Controllers/AGX_LockController.h"
#include "Constraints/Controllers/AGX_RangeController.h"
#include "Constraints/Controllers/AGX_TargetSpeedController.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_WheelJointComponent.generated.h"

class FWheelJointBarrier;

/**
 * Note: the first Rigid Body should be the wheel, and the second the chassis.
 *
 * The wheel joint is designed to attach two bodies allowing free rotation around one axis, the
 * wheel axle. A second axis becomes the steering axis and is also the direction for suspension.
 * This allows for rotation about the center of the wheel around the axle as well as rotation about
 * the steering axis. The wheel is also allowed to move up/down along the steering axis which acts
 * as suspension. By default, there is no control on steering or driving. Suspension is initially
 * locked. By using the secondary constraints, the relative angles can be computed and the extension
 * for suspension can be controlled.
 */
UCLASS(
	ClassGroup = "AGX_Vehicle", Category = "AGX", Blueprintable,
	Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_WheelJointComponent : public UAGX_ConstraintComponent
{
	GENERATED_BODY()

public:
	using FBarrierType = FWheelJointBarrier;

public:
	UAGX_WheelJointComponent();
	virtual ~UAGX_WheelJointComponent() override;

	/**
	 * Get the current angle of the WheelJoint.
	 * This function is only valid during runtime and will return 0.0 otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wheel Joint")
	double GetAngle() const;

	// Steering.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraint|Steering")
	FAGX_ConstraintLockController SteeringLockController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraint|Steering")
	FAGX_ConstraintRangeController SteeringRangeController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraint|Steering")
	FAGX_ConstraintTargetSpeedController SteeringTargetSpeedController;

	// Wheel.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraint|Wheel")
	FAGX_ConstraintLockController WheelLockController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraint|Wheel")
	FAGX_ConstraintRangeController WheelRangeController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraint|Wheel")
	FAGX_ConstraintTargetSpeedController WheelTargetSpeedController;

	// Suspension.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraint|Suspension")
	FAGX_ConstraintLockController SuspensionLockController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraint|Suspension")
	FAGX_ConstraintRangeController SuspensionRangeController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraint|Suspension")
	FAGX_ConstraintTargetSpeedController SuspensionTargetSpeedController;

	// SteeringBounds.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraint|SteeringBounds")
	FAGX_ConstraintLockController SteeringBoundsLockController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraint|SteeringBounds")
	FAGX_ConstraintRangeController SteeringBoundsRangeController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Secondary Constraint|SteeringBounds")
	FAGX_ConstraintTargetSpeedController SteeringBoundsTargetSpeedController;

	FWheelJointBarrier* GetNativeWheelJoint();
	const FWheelJointBarrier* GetNativeWheelJoint() const;

	virtual void CopyFrom(const FConstraintBarrier& Barrier, FAGX_ImportContext* Context) override;

private:
	virtual void CreateNativeImpl() override;
	virtual void UpdateNativeProperties() override;
};
