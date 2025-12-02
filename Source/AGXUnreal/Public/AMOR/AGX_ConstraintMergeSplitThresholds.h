// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "AMOR/ConstraintMergeSplitThresholdsBarrier.h"
#include "AMOR/AGX_MergeSplitThresholdsBase.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ConstraintMergeSplitThresholds.generated.h"

/*
 * Defines the thresholds used by AMOR (merge split) for Constraints, affecting under which
 * conditions Constraints will merge and split.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", BlueprintType, Blueprintable)
class AGXUNREAL_API UAGX_ConstraintMergeSplitThresholds : public UAGX_MergeSplitThresholdsBase
{
	GENERATED_BODY()

public:
	/**
	 * Maximum difference the 'force range'/desired force range parameter in any controller
	 * may change without splitting the constrained objects [N].
	 */
	UPROPERTY(EditAnywhere, Category = "Constraint Merge Split Thresholds")
	FAGX_Real MaxDesiredForceRangeDiff {0.1};

	UFUNCTION(BlueprintCallable, Category = "Constraint Merge Split Thresholds")
	void SetMaxDesiredForceRangeDiff(double InMaxDesiredForceRangeDiff);

	UFUNCTION(BlueprintCallable, Category = "Constraint Merge Split Thresholds")
	double GetMaxDesiredForceRangeDiff() const;

	/**
	 * Maximum difference the 'position'/desired angle parameter in a lock controller may
	 * change without splitting the constrained objects.
	 * For rotational constraints, this is expressed in [deg].
	 * For prismatic constraints, this is expressed in [cm].
	 *
	 * For Cylindrical constraints, this value is converted to [rad] and applied to both the
	 * rotational and translational DOFs of the AGX Dynamics object without further scaling.
	 */
	UPROPERTY(EditAnywhere, Category = "Constraint Merge Split Thresholds")
	FAGX_Real MaxDesiredLockAngleDiff {0.00001};

	UFUNCTION(BlueprintCallable, Category = "Constraint Merge Split Thresholds")
	void SetMaxDesiredLockAngleDiff(double InMaxDesiredLockAngleDiff);

	UFUNCTION(BlueprintCallable, Category = "Constraint Merge Split Thresholds")
	double GetMaxDesiredLockAngleDiff() const;

	/**
	 * Maximum difference the 'position'/desired angle parameter in a range controller may
	 * change without splitting the constrained objects.
	 * For rotational constraints, this is expressed in [deg].
	 * For prismatic constraints, this is expressed in [cm].
	 *
	 * For Cylindrical constraints, this value is converted to [rad] and applied to both the
	 * rotational and translational DOFs of the AGX Dynamics object without further scaling.
	 */
	UPROPERTY(EditAnywhere, Category = "Constraint Merge Split Thresholds")
	FAGX_Real MaxDesiredRangeAngleDiff {0.00001};

	UFUNCTION(BlueprintCallable, Category = "Constraint Merge Split Thresholds")
	void SetMaxDesiredRangeAngleDiff(double InMaxDesiredRangeAngleDiff);

	UFUNCTION(BlueprintCallable, Category = "Constraint Merge Split Thresholds")
	double GetMaxDesiredRangeAngleDiff() const;

	/**
	 * Maximum difference the 'speed'/desired speed parameter in a motor controller may
	 * change without splitting the constrained objects.
	 * For rotational constraints, this is expressed in [deg/s].
	 * For prismatic constraints, this is expressed in [cm/s].
	 *
	 * For Cylindrical constraints, this value is converted to [rad/s] and applied to both the
	 * rotational and translational DOFs of the AGX Dynamics object without further scaling.
	 */
	UPROPERTY(EditAnywhere, Category = "Constraint Merge Split Thresholds")
	FAGX_Real MaxDesiredSpeedDiff {0.00001};

	UFUNCTION(BlueprintCallable, Category = "Constraint Merge Split Thresholds")
	void SetMaxDesiredSpeedDiff(double InMaxDesiredSpeedDiff);

	UFUNCTION(BlueprintCallable, Category = "Constraint Merge Split Thresholds")
	double GetMaxDesiredSpeedDiff() const;

	/**
	 * Maximum relative speed between the constrained objects for the system to be considered at
	 * rest.
	 * For rotational constraints, this is expressed in [deg/s].
	 * For prismatic constraints, this is expressed in [cm/s].
	 *
	 * For Cylindrical constraints, this value is converted to [rad/s] and applied to both the
	 * rotational and translational DOFs of the AGX Dynamics object without further scaling.
	 */
	UPROPERTY(EditAnywhere, Category = "Constraint Merge Split Thresholds")
	FAGX_Real MaxRelativeSpeed {0.005};

	UFUNCTION(BlueprintCallable, Category = "Constraint Merge Split Thresholds")
	void SetMaxRelativeSpeed(double InMaxRelativeSpeed);

	UFUNCTION(BlueprintCallable, Category = "Constraint Merge Split Thresholds")
	double GetMaxRelativeSpeed() const;

	void CreateNative(bool bIsRotational);
	bool HasNative() const;
	FConstraintMergeSplitThresholdsBarrier* GetOrCreateNative(
		UWorld* PlayingWorld, bool bIsRotational);
	FConstraintMergeSplitThresholdsBarrier* GetNative();
	const FConstraintMergeSplitThresholdsBarrier* GetNative() const;

	static UAGX_ConstraintMergeSplitThresholds* CreateFromAsset(
		UWorld* PlayingWorld, UAGX_ConstraintMergeSplitThresholds& Source, bool bIsRotational);

	UAGX_ConstraintMergeSplitThresholds* GetOrCreateInstance(
		UWorld* PlayingWorld, bool bIsRotational);

	UAGX_ConstraintMergeSplitThresholds* GetInstance();

	bool IsInstance() const;

	void CopyFrom(const FMergeSplitThresholdsBarrier& Barrier);

	/*
	 * Assigns the property values of this class to the passed barrier.
	 */
	void CopyTo(FConstraintMergeSplitThresholdsBarrier& Barrier);

public: // Deprecated functions.
	UFUNCTION(
		BlueprintCallable, Category = "Constraint Merge Split Thresholds",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage =
				 "Use SetMaxDesiredForceRangeDiff instead of SetMaxDesiredForceRangeDiff_BP"))
	void SetMaxDesiredForceRangeDiff_BP(float InMaxDesiredForceRangeDiff);

	UFUNCTION(
		BlueprintCallable, Category = "Constraint Merge Split Thresholds",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage =
				 "Use GetMaxDesiredForceRangeDiff instead of GetMaxDesiredForceRangeDiff_BP"))
	float GetMaxDesiredForceRangeDiff_BP() const;

	UFUNCTION(
		BlueprintCallable, Category = "Constraint Merge Split Thresholds",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage =
				 "Use SetMaxDesiredLockAngleDiff instead of SetMaxDesiredLockAngleDiff_BP"))
	void SetMaxDesiredLockAngleDiff_BP(float InMaxDesiredLockAngleDiff);

	UFUNCTION(
		BlueprintCallable, Category = "Constraint Merge Split Thresholds",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage =
				 "Use GetMaxDesiredLockAngleDiff instead of GetMaxDesiredLockAngleDiff_BP"))
	float GetMaxDesiredLockAngleDiff_BP() const;

	UFUNCTION(
		BlueprintCallable, Category = "Constraint Merge Split Thresholds",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage =
				 "Use SetMaxDesiredRangeAngleDiff instead of SetMaxDesiredRangeAngleDiff_BP"))
	void SetMaxDesiredRangeAngleDiff_BP(float InMaxDesiredRangeAngleDiff);

	UFUNCTION(
		BlueprintCallable, Category = "Constraint Merge Split Thresholds",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage =
				 "Use GetMaxDesiredRangeAngleDiff instead of GetMaxDesiredRangeAngleDiff_BP"))
	float GetMaxDesiredRangeAngleDiff_BP() const;

	UFUNCTION(
		BlueprintCallable, Category = "Constraint Merge Split Thresholds",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage =
				 "Use SetMaxDesiredSpeedDiff instead of SetMaxDesiredSpeedDiff_BP"))
	void SetMaxDesiredSpeedDiff_BP(float InMaxDesiredSpeedDiff);

	UFUNCTION(
		BlueprintCallable, Category = "Constraint Merge Split Thresholds",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage =
				 "Use GetMaxDesiredSpeedDiff instead of GetMaxDesiredSpeedDiff_BP"))
	float GetMaxDesiredSpeedDiff_BP() const;

	UFUNCTION(
		BlueprintCallable, Category = "Constraint Merge Split Thresholds",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use SetMaxRelativeSpeed instead of SetMaxRelativeSpeed_BP"))
	void SetMaxRelativeSpeed_BP(float InMaxRelativeSpeed);

	UFUNCTION(
		BlueprintCallable, Category = "Constraint Merge Split Thresholds",
		Meta =
			(DeprecatedFunction,
			 DeprecationMessage = "Use GetMaxRelativeSpeed instead of GetMaxRelativeSpeed_BP"))
	float GetMaxRelativeSpeed_BP() const;

private:
#if WITH_EDITOR
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	void InitPropertyDispatcher();
#endif

	void SetNativeProperties();
	void CopyFrom(const UAGX_ConstraintMergeSplitThresholds& Source);

	TWeakObjectPtr<UAGX_ConstraintMergeSplitThresholds> Asset;
	TWeakObjectPtr<UAGX_ConstraintMergeSplitThresholds> Instance;
	FConstraintMergeSplitThresholdsBarrier NativeBarrier;
};
