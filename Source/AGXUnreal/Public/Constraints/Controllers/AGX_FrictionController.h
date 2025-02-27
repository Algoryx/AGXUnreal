// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_ConstraintController.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_FrictionController.generated.h"

class FFrictionControllerBarrier;

/**
 * Friction controller for secondary constraints (usually on one of the DOFs
 * that has not been primarily constrained by the AGX Constraint). Disabled by
 * default.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ConstraintFrictionController : public FAGX_ConstraintController
{
	GENERATED_BODY()

	/**
	 * Note that if this controller is rotational (Hinge or CylindriclJoint)
	 * the radius (meters) of the axle should be included in the friction coefficient
	 * for the comparisons with the normal force to be dimensionally correct.
	 * I.e., friction_torque <= friction_coefficient * axle_radius * normal_force
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Friction Controller", Meta = (EditCondition = "bEnable"))
	double FrictionCoefficient {0.416667};

	void SetFrictionCoefficient(double InFrictionCoefficient);

	double GetFrictionCoefficient() const;

	/**
	 * Enable/disable non-linear update of the friction conditions given
	 * current normal force from the direct solver. When enabled - this
	 * feature is similar to scale box friction models with solve type DIRECT.
	 *
	 * Note that this feature only supports constraint solve types DIRECT and
	 * DIRECT_AND_ITERATIVE - meaning, if the constraint has solve
	 * type ITERATIVE, this feature is ignored.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Friction Controller", Meta = (EditCondition = "bEnable"))
	bool bEnableNonLinearDirectSolveUpdate {false};

	void SetEnableNonLinearDirectSolveUpdate(bool bInEnableNonLinearDirectSolveUpdate);

	bool GetEnableNonLinearDirectSolveUpdate() const;

public:
	FAGX_ConstraintFrictionController() = default;
	FAGX_ConstraintFrictionController(bool bRotational);

	void InitializeBarrier(TUniquePtr<FFrictionControllerBarrier> Barrier);
	void CopyFrom(const FFrictionControllerBarrier& Source);

private:
	virtual void UpdateNativePropertiesImpl() override;
};

/**
 * This class acts as an API that exposes functions of FAGX_TargetSpeedController in Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_ConstraintFrictionController_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetFrictionCoefficient(
		UPARAM(ref) FAGX_ConstraintFrictionController& Controller, float FrictionCoefficient)
	{
		Controller.SetFrictionCoefficient(static_cast<double>(FrictionCoefficient));
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetFrictionCoefficient(UPARAM(ref) FAGX_ConstraintFrictionController& Controller)
	{
		return static_cast<float>(Controller.GetFrictionCoefficient());
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetEnableNonLinearDirectSolveUpdate(
		UPARAM(ref) FAGX_ConstraintFrictionController& Controller, bool bEnable)
	{
		Controller.SetEnableNonLinearDirectSolveUpdate(bEnable);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static bool GetEnableNonLinearDirectSolveUpdate(
		UPARAM(ref) FAGX_ConstraintFrictionController& Controller)
	{
		return Controller.GetEnableNonLinearDirectSolveUpdate();
	}

	//~ Begin AGX_ConstraintController Blueprint Library interface.
	// These are copy/pasted from FAGX_ConstraintController.h. See the comment in that file.

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static bool IsValid(UPARAM(ref) FAGX_ConstraintFrictionController& ControllerRef)
	{
		return ControllerRef.HasNative();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetEnable(UPARAM(ref) FAGX_ConstraintFrictionController& ControllerRef, bool Enable)
	{
		return ControllerRef.SetEnable(Enable);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static bool GetEnable(UPARAM(ref) FAGX_ConstraintFrictionController& ControllerRef)
	{
		return ControllerRef.GetEnable();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetCompliance(
		UPARAM(ref) FAGX_ConstraintFrictionController& Controller, float Compliance)
	{
		Controller.SetCompliance(static_cast<double>(Compliance));
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetCompliance(UPARAM(ref) const FAGX_ConstraintFrictionController& Controller)
	{
		return static_cast<float>(Controller.GetCompliance());
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetSpookDamping(
		UPARAM(ref) FAGX_ConstraintFrictionController& Controller, float SpookDamping)
	{
		Controller.SetSpookDamping(static_cast<double>(SpookDamping));
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetSpookDamping(UPARAM(ref) const FAGX_ConstraintFrictionController& Controller)
	{
		return static_cast<float>(Controller.GetSpookDamping());
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetForceRange(
		UPARAM(ref) FAGX_ConstraintFrictionController& Controller, float MinForce, float MaxForce)
	{
		Controller.SetForceRange(MinForce, MaxForce);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForceRangeMin(UPARAM(ref) const FAGX_ConstraintFrictionController& Controller)
	{
		return Controller.GetForceRange().Min;
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForceRangeMax(UPARAM(ref) const FAGX_ConstraintFrictionController& Controller)
	{
		return Controller.GetForceRange().Max;
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForce(UPARAM(ref) FAGX_ConstraintFrictionController& ControllerRef)
	{
		return static_cast<float>(ControllerRef.GetForce());
	}

	//~ End AGX_ConstraintController Blueprint Library interface.
};
