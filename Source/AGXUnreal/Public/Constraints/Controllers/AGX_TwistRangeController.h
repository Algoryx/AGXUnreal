// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_RealInterval.h"
#include "Constraints/AGX_ElementaryConstraint.h"
#include "Constraints/TwistRangeControllerBarrier.h"

// Unreal Engine includes.
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_TwistRangeController.generated.h"

/**
 * A Twist Range Controller limits the range of relative rotation between two Rigid Bodies. It is
 * only available on Constraint Components that allow for multiple axes of rotation of which one
 * is the twist axis. Ball Constraint is an example of such a Constraint Component. Hinge is not
 * since it only allows for rotation around a single axis and therefore has a Range Controller
 * instead.
 *
 * Twist is defined to be rotation around the constraint's Z axis.
 *
 * This type cannot be used to create new native AGX Dynamics instances of the Twist Range
 * Controller, it is only used to access a Twist Range Controller that exists on a constraint such
 * as Ball Constraint.
 *
 */
USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_TwistRangeController : public FAGX_ElementaryConstraint
{
	GENERATED_BODY()

public: // Special member functions.
	FAGX_TwistRangeController();
	virtual ~FAGX_TwistRangeController();

public: // Properties.
	/**
	 * The amount of rotation around the constraint's Z axis that is allowed [deg].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Twist Range Controller", Meta = (EditCondition = "bEnable"))
	FAGX_RealInterval Range;

	void SetRange(FDoubleInterval InRange);
	void SetRange(FAGX_RealInterval InRange);
	void SetRange(double InRangeMin, double InRangeMax);
	void SetRangeMin(double InMin);
	void SetRangeMax(double InMax);

	FDoubleInterval GetRange() const;
	double GetRangeMin() const;
	double GetRangeMax() const;

public: // Native management.
	bool HasNative() const;
	FTwistRangeControllerBarrier* GetNative();
	const FTwistRangeControllerBarrier* GetNative() const;

	/**
	 * Bind this Twist Range Controller to the AGX Dynamics Twist Range Controller referenced by
	 * the given Barrier.
	 */
	void InitializeBarrier(const FTwistRangeControllerBarrier& InBarrier);

	/**
	 * Copy property values from the given Source Barrier into this.
	 *
	 * @param Source The Barrier to read through.
	 * @param ArchetypeInstances Template instances to update.
	 * @param bForceOverwriteInstances Whether to also update instances with edited values.
	 */
	void CopyFrom(const FTwistRangeControllerBarrier& Source);

public: // Member function overrides.
	//~ Begin FAGX_ElementaryConstraint interface.
	virtual void UpdateNativeProperties() override;
	virtual void InitializeBarrier(const FElementaryConstraintBarrier& InBarrier) override;
	//~ End FAGX_ElementaryConstraint interface.

private:
	FTwistRangeControllerBarrier Barrier;
};

/**
 * This class acts as an API that exposes functions of FAGX_TwistRangeController in Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_TwistRangeController_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/**
	 * Set the range that the two constrained bodies may twist relative to each other [deg].
	 * @param Controller The Twist Range Controller to set range on.
	 * @param Min The minimum allowed angle [deg].
	 * @param Max The maximum allowed angle [deg].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Twist Range Controller")
	static void SetRange(UPARAM(Ref) FAGX_TwistRangeController& Controller, double Min, double Max)
	{
		Controller.SetRange(Min, Max);
	}

	/**
	 * Get the range of angles that the two constrained bodies may twist relative to each other
	 * [deg].
	 * @param Controller The Twist Range Controller to get allowed angle range from.
	 * @param Min The minimum angle allowed [deg].
	 * @param Max The maximum angle allowed [deg].
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Twist Range Controller")
	static void GetRange(const FAGX_TwistRangeController& Controller, double& Min, double& Max)
	{
		Min = Controller.GetRangeMin();
		Max = Controller.GetRangeMax();
	}

	// Add Elementary Constraint API. See comment in AGX_ElementaryConstraint.h.

	/**
	 * Enable or disable an Elementary Constraint.
	 * @param Constraint The Elementary Constraint to enable or disable.
	 * @param Enable True to enable the Elementary Constraint, false to disable it.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void SetEnable(UPARAM(Ref) FAGX_TwistRangeController& Constraint, bool Enable)
	{
		return Constraint.SetEnable(Enable);
	}

	/**
	 * Check whether the given Elementary Constraint is enabled or disabled.
	 * @param Constraint The Elementary Constraint to check.
	 * @return True if the Elementary Constraint is enabled, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static bool GetEnable(UPARAM(Ref) FAGX_TwistRangeController& Constraint)
	{
		return Constraint.GetEnable();
	}

	/**
	 * Set the compliance of an Elementary Constraint. [m/N] for translational DOFs and [rad/Nm] for
	 * rotational DOFs.
	 * @param Constraint The Elementary Constraint to set Compliance on.
	 * @param Compliance The new Compliance.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void SetCompliance(UPARAM(Ref) FAGX_TwistRangeController& Constraint, double Compliance)
	{
		return Constraint.SetCompliance(Compliance);
	}

	/**
	 * Get the Compliance from othe Elementary Constraint. [m/N] for translational DOFs and [rad/Nm]
	 * for rotational DOFs.
	 * @param Constraint The Elementary Constraint to get Compliance from.
	 * @return The Compliance of the Elementary Constraint.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static double GetCompliance(UPARAM(Ref) FAGX_TwistRangeController& Constraint)
	{
		return Constraint.GetCompliance();
	}

	/**
	 * Set the Spook damping time of an Elementary Constraint [s].
	 * @param Constraint Elementary Constraint to set Spook Damping on.
	 * @param SpookDamping New Spook damping time.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void SetSpookDamping(
		UPARAM(Ref) FAGX_TwistRangeController& Constraint, double SpookDamping)
	{
		return Constraint.SetSpookDamping(SpookDamping);
	}

	/**
	 * Get the Spook damping time from the Elementary Constraint [s].
	 * @param Constraint The Elementary Constraint to get Spook damping from.
	 * @return The Spook damping of the Elementary Constraint.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static double GetSpookDamping(UPARAM(Ref) FAGX_TwistRangeController& Constraint)
	{
		return Constraint.GetSpookDamping();
	}

	/**
	 * Set the Force Range of an Elementary Constraint. The force applied by the solver will always
	 * be within this bound [N] or [Nm].
	 * @param Constraint Constraint to set Force Range on.
	 * @param Min Minimum force or torque the Elementary Constraint may apply [N] or [Nm].
	 * @param Max Maximum force or torque the Elementary Constraint may apply [N] or [Nm].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void SetForceRange(
		UPARAM(Ref) FAGX_TwistRangeController& Constraint, double Min, double Max)
	{
		return Constraint.SetForceRange(Min, Max);
	}

	/**
	 * Set the minimum force or torque an Elementary Constraint can apply [N] or [Nm].
	 * @param Constraint The Elementary Constraint to set minimum force range on.
	 * @param Min The new minimum force or torque [N] or [Nm].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void SetForceRangeMin(UPARAM(Ref) FAGX_TwistRangeController& Constraint, double Min)
	{
		Constraint.SetForceRangeMin(Min);
	}

	/**
	 * Set the maximum force or torque an Elementary Constraint can apply [N] or [Nm].
	 * @param Constraint The Elementary Constraint to set maximum force range on.
	 * @param Max The new maximum force or torque [N] or [Nm].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void SetForceRangeMax(UPARAM(Ref) FAGX_TwistRangeController& Constraint, double Max)
	{
		Constraint.SetForceRangeMax(Max);
	}

	/**
	 * Get the Force Range of the Elementary Constraint  [N] or [Nm].
	 * @param Constraint The Elementary Constraint to get the Force Range for.
	 * @param Min The minimum force or torque the constraint can apply [N] or [Nm].
	 * @param Max The maximum force or torque the constraint can apply [N] or [Nm].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void GetForceRange(
		UPARAM(Ref) FAGX_TwistRangeController& Constraint, double& Min, double& Max)
	{
		FDoubleInterval ForceRange = Constraint.GetForceRange();
		Min = ForceRange.Min;
		Max = ForceRange.Max;
	}

	/**
	 * Get the minimum force or torque the Elementary Constraint can apply [N] or [Nm].
	 * @param Constraint The Elementary Constraint to get the minimum force range for.
	 * @return The minimum force or torque the Elementary Constraint can apply [N] or [Nm].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static double GetForceRangeMin(UPARAM(Ref) FAGX_TwistRangeController& Constraint)
	{
		return Constraint.GetForceRangeMin();
	}

	/**
	 * Get the maximum force or torque the Elementary Constraint can apply [N] or [Nm].
	 * @param Constraint The Elementary Constraint to get the maximum force range for.
	 * @return The maximum force or torque the Elementary Constraint can apply [N] or [Nm].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static double GetForceRangeMax(UPARAM(Ref) FAGX_TwistRangeController& Constraint)
	{
		return Constraint.GetForceRangeMax();
	}
};
