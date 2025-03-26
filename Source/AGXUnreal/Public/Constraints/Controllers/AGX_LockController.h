// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_ConstraintController.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_LockController.generated.h"

class FLockControllerBarrier;

/**
 * Lock controller for secondary constraints (usually on one of the DOFs
 * that has not been primarily constrained by the AGX Constraint).
 * Disabled by default.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ConstraintLockController : public FAGX_ConstraintController
{
	GENERATED_BODY()

	/**
	 * Target position in degrees [deg] if controller is on a Rotational
	 * Degree-Of-Freedom, else in centimeters[cm].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Lock Controller", Meta = (EditCondition = "bEnable"))
	double Position {0.0};

	void SetPosition(double InPosisiton);
	double GetPosition() const;

public:
	FAGX_ConstraintLockController() = default;
	FAGX_ConstraintLockController(bool bRotational);

	void InitializeBarrier(TUniquePtr<FLockControllerBarrier> Barrier);
	void CopyFrom(const FLockControllerBarrier& Source);

protected:
	virtual void UpdateNativePropertiesImpl() override;
};

/**
 * This class acts as an API that exposes functions of FAGX_TargetSpeedController in Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_ConstraintLockController_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "AGX Lock Controller")
	static void SetPosition(
		UPARAM(ref) FAGX_ConstraintLockController& ControllerRef, const float Position)
	{
		ControllerRef.SetPosition(static_cast<double>(Position));
	};

	UFUNCTION(BlueprintCallable, Category = "AGX Lock Controller")
	static float GetPosition(UPARAM(ref) FAGX_ConstraintLockController& ControllerRef)
	{
		return static_cast<float>(ControllerRef.GetPosition());
	};

	//~ Begin AGX_ConstraintController Blueprint Library interface.
	// These are copy/pasted from FAGX_ConstraintController.h. See the comment in that file.

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static bool IsValid(UPARAM(ref) FAGX_ConstraintLockController& ControllerRef)
	{
		return ControllerRef.HasNative();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetEnable(UPARAM(ref) FAGX_ConstraintLockController& ControllerRef, bool Enable)
	{
		return ControllerRef.SetEnable(Enable);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static bool GetEnable(UPARAM(ref) FAGX_ConstraintLockController& ControllerRef)
	{
		return ControllerRef.GetEnable();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetCompliance(
		UPARAM(ref) FAGX_ConstraintLockController& Controller, float Compliance)
	{
		Controller.SetCompliance(static_cast<double>(Compliance));
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetCompliance(UPARAM(ref) const FAGX_ConstraintLockController& Controller)
	{
		return static_cast<float>(Controller.GetCompliance());
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetSpookDamping(
		UPARAM(ref) FAGX_ConstraintLockController& Controller, float SpookDamping)
	{
		Controller.SetSpookDamping(static_cast<double>(SpookDamping));
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetSpookDamping(UPARAM(ref) const FAGX_ConstraintLockController& Controller)
	{
		return static_cast<float>(Controller.GetSpookDamping());
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetForceRange(
		UPARAM(ref) FAGX_ConstraintLockController& Controller, float MinForce, float MaxForce)
	{
		Controller.SetForceRange(MinForce, MaxForce);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForceRangeMin(UPARAM(ref) const FAGX_ConstraintLockController& Controller)
	{
		return Controller.GetForceRange().Min;
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForceRangeMax(UPARAM(ref) const FAGX_ConstraintLockController& Controller)
	{
		return Controller.GetForceRange().Max;
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForce(UPARAM(ref) FAGX_ConstraintLockController& ControllerRef)
	{
		return static_cast<float>(ControllerRef.GetForce());
	}

	//~ End AGX_ConstraintController Blueprint Library interface.
};
