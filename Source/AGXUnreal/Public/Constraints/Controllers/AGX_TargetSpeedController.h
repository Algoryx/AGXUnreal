// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_ConstraintController.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_TargetSpeedController.generated.h"

class FTargetSpeedControllerBarrier;

/**
 * Target speed controller for secondary constraints (usually on one of the DOFs
 * that has not been primarily constrained by the AGX Constraint). Disabled by
 * default.
 */
USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_ConstraintTargetSpeedController : public FAGX_ConstraintController
{
	GENERATED_BODY()

	/**
	 * Target Speed in Degrees Per Second [deg/s] if controller is on a Rotational DOF,
	 * else in Centimeters Per Second [cm/s].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Target Speed Controller", Meta = (EditCondition = "bEnable"))
	double Speed {0.0};

	void SetSpeed(double InSpeed);
	double GetSpeed() const;

	/**
	 * Whether the controller should auto-lock whenever target speed is zero,
	 * such that it will not drift away from that angle/position if the assigned
	 * force range is enough to hold it.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Target Speed Controller", Meta = (EditCondition = "bEnable"))
	bool bLockedAtZeroSpeed {false};

	void SetLockedAtZeroSpeed(bool bInLockedAtZeroSpeed);
	bool GetLockedAtZeroSpeed() const;

public:
	FAGX_ConstraintTargetSpeedController() = default;
	FAGX_ConstraintTargetSpeedController(bool bRotational);

	void InitializeBarrier(TUniquePtr<FTargetSpeedControllerBarrier> Barrier);
	void CopyFrom(const FTargetSpeedControllerBarrier& Source);

private:
	virtual void UpdateNativePropertiesImpl() override;
};

/**
 * This class acts as an API that exposes functions of FAGX_TargetSpeedController in Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_ConstraintTargetSpeedController_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "AGX Target Speed Controller")
	static void SetSpeed(
		UPARAM(ref) FAGX_ConstraintTargetSpeedController& Controller, const float Speed)
	{
		Controller.SetSpeed(static_cast<double>(Speed));
	};

	UFUNCTION(BlueprintCallable, Category = "AGX Target Speed Controller")
	static float GetSpeed(UPARAM(ref) FAGX_ConstraintTargetSpeedController& Controller)
	{
		return static_cast<float>(Controller.GetSpeed());
	};

	UFUNCTION(BlueprintCallable, Category = "AGX Target Speed Controller")
	static void SetLockedAtZeroSpeed(
		UPARAM(ref) FAGX_ConstraintTargetSpeedController& Controller, bool bInLockedAtZeroSpeed)
	{
		Controller.SetLockedAtZeroSpeed(bInLockedAtZeroSpeed);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Target Speed Controller")
	static bool GetLocedAtZeroSpeed(UPARAM(ref) FAGX_ConstraintTargetSpeedController& Controller)
	{
		return Controller.GetLockedAtZeroSpeed();
	}

	//~ Begin AGX_ConstraintController Blueprint Library interface.
	// These are copy/pasted from FAGX_ConstraintController.h. See the comment in that file.

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static bool IsValid(UPARAM(ref) FAGX_ConstraintTargetSpeedController& Controller)
	{
		return Controller.HasNative();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetEnable(UPARAM(ref) FAGX_ConstraintTargetSpeedController& Controller, bool Enable)
	{
		return Controller.SetEnable(Enable);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static bool GetEnable(UPARAM(ref) FAGX_ConstraintTargetSpeedController& Controller)
	{
		return Controller.GetEnable();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetCompliance(
		UPARAM(ref) FAGX_ConstraintTargetSpeedController& Controller, float Compliance)
	{
		Controller.SetCompliance(static_cast<double>(Compliance));
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetCompliance(UPARAM(ref) const FAGX_ConstraintTargetSpeedController& Controller)
	{
		return static_cast<float>(Controller.GetCompliance());
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetSpookDamping(
		UPARAM(ref) FAGX_ConstraintTargetSpeedController& Controller, float SpookDamping)
	{
		Controller.SetSpookDamping(static_cast<double>(SpookDamping));
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetSpookDamping(UPARAM(ref) const FAGX_ConstraintTargetSpeedController& Controller)
	{
		return static_cast<float>(Controller.GetSpookDamping());
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetForceRange(
		UPARAM(ref) FAGX_ConstraintTargetSpeedController& Controller, float MinForce,
		float MaxForce)
	{
		Controller.SetForceRange(MinForce, MaxForce);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForceRangeMin(UPARAM(ref)
									  const FAGX_ConstraintTargetSpeedController& Controller)
	{
		return Controller.GetForceRange().Min;
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForceRangeMax(UPARAM(ref)
									  const FAGX_ConstraintTargetSpeedController& Controller)
	{
		return Controller.GetForceRange().Max;
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForce(UPARAM(ref) FAGX_ConstraintTargetSpeedController& Controller)
	{
		return static_cast<float>(Controller.GetForce());
	}

	//~ End AGX_ConstraintController Blueprint Library interface.
};
