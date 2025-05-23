// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_RealInterval.h"
#include "Constraints/AGX_ConstraintController.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_RangeController.generated.h"

class FRangeControllerBarrier;

/**
 * Range controller for secondary constraints (usually on one of the DOFs
 * that has not been primarily constrained by the AGX Constraint).
 * Disabled by default.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ConstraintRangeController : public FAGX_ConstraintController
{
	GENERATED_BODY()

	/**
	 * Range in Degrees [deg] if controller is on a Rotational Degree-Of-Freedom,
	 * else in Centimeters [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Range Controller", Meta = (EditCondition = "bEnable"))
	FAGX_RealInterval Range;

	void SetRange(const FAGX_RealInterval& InRange);

	void SetRange(double RangeMin, double RangeMax);

	FAGX_RealInterval GetRange() const;

public:
	FAGX_ConstraintRangeController();

	void InitializeBarrier(TUniquePtr<FRangeControllerBarrier> Barrier);
	void CopyFrom(const FRangeControllerBarrier& Source);

private:
	virtual void UpdateNativePropertiesImpl() override;
};

/**
 * This class acts as an API that exposes functions of FAGX_TargetSpeedController in Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_ConstraintRangeController_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "AGX Range Controller")
	static void SetRange(
		UPARAM(ref) FAGX_ConstraintRangeController& Controller, float RangeMin, float RangeMax)
	{
		Controller.SetRange(RangeMin, RangeMax);
	};

	UFUNCTION(BlueprintCallable, Category = "AGX Range Controller")
	static float GetRangeMax(UPARAM(ref) FAGX_ConstraintRangeController& Controller)
	{
		return Controller.GetRange().Max;
	};

	UFUNCTION(BlueprintCallable, Category = "AGX Range Controller")
	static float GetRangeMin(UPARAM(ref) FAGX_ConstraintRangeController& Controller)
	{
		return Controller.GetRange().Min;
	};

	//~ Begin AGX_ConstraintController Blueprint Library interface.
	// These are copy/pasted from FAGX_ConstraintController.h. See the comment in that file.

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static bool IsValid(UPARAM(ref) FAGX_ConstraintRangeController& ControllerRef)
	{
		return ControllerRef.HasNative();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetEnable(UPARAM(ref) FAGX_ConstraintRangeController& ControllerRef, bool Enable)
	{
		return ControllerRef.SetEnable(Enable);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static bool GetEnable(UPARAM(ref) FAGX_ConstraintRangeController& ControllerRef)
	{
		return ControllerRef.GetEnable();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetCompliance(
		UPARAM(ref) FAGX_ConstraintRangeController& Controller, float Compliance)
	{
		Controller.SetCompliance(static_cast<double>(Compliance));
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetCompliance(UPARAM(ref) const FAGX_ConstraintRangeController& Controller)
	{
		return static_cast<float>(Controller.GetCompliance());
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetElasticity(
		UPARAM(ref) FAGX_ConstraintRangeController& Controller, float Elasticity)
	{
		Controller.SetElasticity(static_cast<double>(Elasticity));
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetElasticity(UPARAM(ref) const FAGX_ConstraintRangeController& Controller)
	{
		return static_cast<float>(Controller.GetElasticity());
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetSpookDamping(
		UPARAM(ref) FAGX_ConstraintRangeController& Controller, float SpookDamping)
	{
		Controller.SetSpookDamping(static_cast<double>(SpookDamping));
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetSpookDamping(UPARAM(ref) const FAGX_ConstraintRangeController& Controller)
	{
		return static_cast<float>(Controller.GetSpookDamping());
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetForceRange(
		UPARAM(ref) FAGX_ConstraintRangeController& Controller, float MinForce, float MaxForce)
	{
		Controller.SetForceRange(MinForce, MaxForce);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForceRangeMin(UPARAM(ref) const FAGX_ConstraintRangeController& Controller)
	{
		return Controller.GetForceRange().Min;
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForceRangeMax(UPARAM(ref) const FAGX_ConstraintRangeController& Controller)
	{
		return Controller.GetForceRange().Max;
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForce(UPARAM(ref) FAGX_ConstraintRangeController& ControllerRef)
	{
		return static_cast<float>(ControllerRef.GetForce());
	}

	//~ End AGX_ConstraintController Blueprint Library interface.
};
