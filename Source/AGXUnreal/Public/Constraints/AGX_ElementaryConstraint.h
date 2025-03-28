// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "AGX_RealInterval.h"
#include "Constraints/AGX_ConstraintConstants.h"
#include "Constraints/ElementaryConstraintBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Math/Interval.h"

#include "AGX_ElementaryConstraint.generated.h"

USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_ElementaryConstraint
{
	GENERATED_BODY()

public: // Special member functions.
	FAGX_ElementaryConstraint();
	virtual ~FAGX_ElementaryConstraint();

public: // Properties and property accessors.
	/**
	 * Whether this Constraint is enabled or not. A disabled Constraint has no effect on the
	 * simulation.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Constraint Controller")
	bool bEnable {true};

	void SetEnable(bool bInEnable);

	bool GetEnable() const;

	/**
	 * The compliance in a certain DOF. Measured in [m/N] for translational DOFs and [rad/Nm] for
	 * rotational DOFs.
	 *
	 * The compliance measure the inverse of stiffness of the Constraint Controller. A smaller
	 * compliance will cause a stronger force or torque to be created to restore from the violation.
	 * A too small compliance will lead to instabilities in the simulation.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Controller",
		Meta =
			(EditCondition = "bEnable", SliderMin = "1e-10", SliderMax = "1",
			 SliderExponent = "100000"))
	FAGX_Real Compliance {ConstraintConstants::DefaultCompliance()};

	void SetCompliance(double InCompliance);

	double GetCompliance() const;

	/**
	 * Elasticity is defined as the inverse of compliance, so setting the elasticity will modify
	 * the compliance and vice versa [N/m] or [Nm/rad].
	 */
	void SetElasticity(double InElasticity);

	double GetElasticity() const;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Controller", Meta = (EditCondition = "bEnable"))
	FAGX_Real SpookDamping {ConstraintConstants::DefaultSpookDamping()};

	void SetSpookDamping(double InSpookDamping);

	double GetSpookDamping() const;

	/**
	 * The minimum and maximum force or torque that the constraint controller can produce [N] or
	 * [Nm].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Controller", Meta = (EditCondition = "bEnable"))
	FAGX_RealInterval ForceRange {ConstraintConstants::DefaultForceRange()};

	void SetForceRange(const FDoubleInterval& InForceRange);
	void SetForceRange(const FAGX_RealInterval& InForceRange);
	void SetForceRange(double InForceRangeMin, double InForceRangeMax);
	void SetForceRangeMin(double InMinForce);
	void SetForceRangeMax(double InMaxForce);

	FDoubleInterval GetForceRange() const;
	double GetForceRangeMin() const;
	double GetForceRangeMax() const;

public: // Runtime-only accessors that read directly from the native.
	/**
	 * Get the force or torque that was applied by this Elementary constraint in the most recent
	 * time step.
	 */
	double GetForce();

	/**
	 * Whether the constraint was active or not during the most recent time step. Most constraints
	 * are active as long as they are enabled but some activate based on simulation state, such as
	 * Range Controller activating when the range is hit.
	 */
	bool IsActive() const;

	/**
	 * Get the AGX Dynamics internal name for this constraint.
	 */
	FString GetName() const;

public: // Native management.
	bool HasNative() const;
	FElementaryConstraintBarrier* GetNative();
	const FElementaryConstraintBarrier* GetNative() const;

	virtual void InitializeBarrier(const FElementaryConstraintBarrier& InBarrier);

	/**
	 * Apply the UProperties on the native AGX Dynamics constraint. May only be called if HasNative
	 * returns true.
	 */
	virtual void UpdateNativeProperties();

	/**
	 * Copy properties from the give AGX Dynamics constraint controller into this AGXUnreal
	 * constraint controller.
	 *
	 * Note that no properties held by child classes will be copied. To do that pass a Barrier of
	 * the child class type.
	 */
	void CopyFrom(const FElementaryConstraintBarrier& Source);

protected:
	FElementaryConstraintBarrier Barrier;
};

/**
 * This class acts as an API that exposes functions of FAGX_ElementaryConstraint in Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_ElementaryConstraint_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/**
	 * Enable or disable an Elementary Constraint.
	 * @param Constraint The Elementary Constraint to enable or disable.
	 * @param Enable True to enable the Elementary Constraint, false to disable it.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void SetEnable(UPARAM(Ref) FAGX_ElementaryConstraint& Constraint, bool Enable)
	{
		return Constraint.SetEnable(Enable);
	}

	/**
	 * Check whether the given Elementary Constraint is enabled or disabled.
	 * @param Constraint The Elementary Constraint to check.
	 * @return True if the Elementary Constraint is enabled, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static bool GetEnable(UPARAM(Ref) FAGX_ElementaryConstraint& Constraint)
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
	static void SetCompliance(UPARAM(Ref) FAGX_ElementaryConstraint& Constraint, double Compliance)
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
	static double GetCompliance(UPARAM(Ref) FAGX_ElementaryConstraint& Constraint)
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
		UPARAM(Ref) FAGX_ElementaryConstraint& Constraint, double SpookDamping)
	{
		return Constraint.SetSpookDamping(SpookDamping);
	}

	/**
	 * Get the Spook damping time from the Elementary Constraint [s].
	 * @param Constraint The Elementary Constraint to get Spook damping from.
	 * @return The Spook damping of the Elementary Constraint.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static double GetSpookDamping(UPARAM(Ref) FAGX_ElementaryConstraint& Constraint)
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
		UPARAM(Ref) FAGX_ElementaryConstraint& Constraint, double Min, double Max)
	{
		return Constraint.SetForceRange(Min, Max);
	}

	/**
	 * Set the minimum force or torque an Elementary Constraint can apply [N] or [Nm].
	 * @param Constraint The Elementary Constraint to set minimum force range on.
	 * @param Min The new minimum force or torque [N] or [Nm].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void SetForceRangeMin(UPARAM(Ref) FAGX_ElementaryConstraint& Constraint, double Min)
	{
		Constraint.SetForceRangeMin(Min);
	}

	/**
	 * Set the maximum force or torque an Elementary Constraint can apply [N] or [Nm].
	 * @param Constraint The Elementary Constraint to set maximum force range on.
	 * @param Max The new maximum force or torque [N] or [Nm].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void SetForceRangeMax(UPARAM(Ref) FAGX_ElementaryConstraint& Constraint, double Max)
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
		UPARAM(Ref) FAGX_ElementaryConstraint& Constraint, double& Min, double& Max)
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
	static double GetForceRangeMin(UPARAM(Ref) FAGX_ElementaryConstraint& Constraint)
	{
		return Constraint.GetForceRangeMin();
	}

	/**
	 * Get the maximum force or torque the Elementary Constraint can apply [N] or [Nm].
	 * @param Constraint The Elementary Constraint to get the maximum force range for.
	 * @return The maximum force or torque the Elementary Constraint can apply [N] or [Nm].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static double GetForceRangeMax(UPARAM(Ref) FAGX_ElementaryConstraint& Constraint)
	{
		return Constraint.GetForceRangeMax();
	}
};

// We have substructs of FAGX_ElementaryConstraint which we also want to have the base struct
// Blueprint Library functions. Unfortunately, we have not found a way to automate this yet. The
// Blueprint Library declared above isn't callable on the subtypes and a #define containing
// all the function declarations and definitions doesn't work because UHT doesn't expand macros. So
// for now we're stuck with copy/paste. The following code block should be copy/pasted in each
// Elementary Constraint sublass' Blueprint Library class. Search/replace FAGX_TYPE and substitute
// the actual Elementary Constraint subtype.
#if 0
	/**
	 * Enable or disable an Elementary Constraint.
	 * @param Constraint The Elementary Constraint to enable or disable.
	 * @param Enable True to enable the Elementary Constraint, false to disable it.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void SetEnable(UPARAM(Ref) FAGX_TYPE& Constraint, bool Enable)
	{
		return Constraint.SetEnable(Enable);
	}

	/**
	 * Check whether the given Elementary Constraint is enabled or disabled.
	 * @param Constraint The Elementary Constraint to check.
	 * @return True if the Elementary Constraint is enabled, false otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static bool GetEnable(UPARAM(Ref) FAGX_TYPE& Constraint)
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
	static void SetCompliance(UPARAM(Ref) FAGX_TYPE& Constraint, double Compliance)
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
	static double GetCompliance(UPARAM(Ref) FAGX_TYPE& Constraint)
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
		UPARAM(Ref) FAGX_TYPE& Constraint, double SpookDamping)
	{
		return Constraint.SetSpookDamping(SpookDamping);
	}

	/**
	 * Get the Spook damping time from the Elementary Constraint [s].
	 * @param Constraint The Elementary Constraint to get Spook damping from.
	 * @return The Spook damping of the Elementary Constraint.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static double GetSpookDamping(UPARAM(Ref) FAGX_TYPE& Constraint)
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
		UPARAM(Ref) FAGX_TYPE& Constraint, double Min, double Max)
	{
		return Constraint.SetForceRange(Min, Max);
	}

	/**
	 * Set the minimum force or torque an Elementary Constraint can apply [N] or [Nm].
	 * @param Constraint The Elementary Constraint to set minimum force range on.
	 * @param Min The new minimum force or torque [N] or [Nm].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void SetForceRangeMin(UPARAM(Ref) FAGX_TYPE& Constraint, double Min)
	{
		Constraint.SetForceRangeMin(Min);
	}

	/**
	 * Set the maximum force or torque an Elementary Constraint can apply [N] or [Nm].
	 * @param Constraint The Elementary Constraint to set maximum force range on.
	 * @param Max The new maximum force or torque [N] or [Nm].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static void SetForceRangeMax(UPARAM(Ref) FAGX_TYPE& Constraint, double Max)
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
		UPARAM(Ref) FAGX_TYPE& Constraint, double& Min, double& Max)
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
	static double GetForceRangeMin(UPARAM(Ref) FAGX_TYPE& Constraint)
	{
		return Constraint.GetForceRangeMin();
	}

	/**
	 * Get the maximum force or torque the Elementary Constraint can apply [N] or [Nm].
	 * @param Constraint The Elementary Constraint to get the maximum force range for.
	 * @return The maximum force or torque the Elementary Constraint can apply [N] or [Nm].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Elementary Constraint")
	static double GetForceRangeMax(UPARAM(Ref) FAGX_TYPE& Constraint)
	{
		return Constraint.GetForceRangeMax();
	}
#endif
