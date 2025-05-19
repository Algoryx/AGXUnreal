// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_ConstraintController.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_ScrewController.generated.h"

class FScrewControllerBarrier;

/**
 * Screw controller that puts a relationship between two free DOFs of a
 * constraint, given that one free DOF is translational and the other free DOF
 * is rotational. Disabled by default.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ConstraintScrewController : public FAGX_ConstraintController
{
	GENERATED_BODY()

	/**
	 * The distance along the screw's axis, that is covered by one complete rotation of the screw
	 * [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Screw Controller", Meta = (EditCondition = "bEnable"))
	double Lead {0.0};

	void SetLead(double InLead);
	double GetLead() const;

public:
	FAGX_ConstraintScrewController() = default;
	FAGX_ConstraintScrewController(bool bRotational);

	void InitializeBarrier(TUniquePtr<FScrewControllerBarrier> Barrier);
	void CopyFrom(const FScrewControllerBarrier& Source);

private:
	virtual void UpdateNativePropertiesImpl() override;
};

/**
 * This class acts as an API that exposes functions of FAGX_TargetSpeedController in Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_ConstraintScrenController_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "AGX Screw Controller")
	static void SetLead(UPARAM(Ref) FAGX_ConstraintScrewController& Controller, const float Lead)
	{
		Controller.SetLead(static_cast<double>(Lead));
	};

	UFUNCTION(BlueprintCallable, Category = "AGX Screw Controller")
	static float GetLead(UPARAM(Ref) FAGX_ConstraintScrewController& Controller)
	{
		return static_cast<float>(Controller.GetLead());
	};

	//~ Begin AGX_ConstraintController Blueprint Library interface.
	// These are copy/pasted from AGX_ConstraintController.h. See the comment in that file.

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static bool IsValid(UPARAM(ref) FAGX_ConstraintScrewController& ControllerRef)
	{
		return ControllerRef.HasNative();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetEnable(UPARAM(ref) FAGX_ConstraintScrewController& ControllerRef, bool Enable)
	{
		return ControllerRef.SetEnable(Enable);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static bool GetEnable(UPARAM(ref) FAGX_ConstraintScrewController& ControllerRef)
	{
		return ControllerRef.GetEnable();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetCompliance(
		UPARAM(ref) FAGX_ConstraintScrewController& Controller, float Compliance)
	{
		Controller.SetCompliance(static_cast<double>(Compliance));
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetCompliance(UPARAM(ref) const FAGX_ConstraintScrewController& Controller)
	{
		return static_cast<float>(Controller.GetCompliance());
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetElasticity(UPARAM(ref) FAGX_ConstraintController& Controller, float Elasticity)
	{
		Controller.SetElasticity(static_cast<double>(Elasticity));
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetElasticity(UPARAM(ref) const FAGX_ConstraintController& Controller)
	{
		return static_cast<float>(Controller.GetElasticity());
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetSpookDamping(
		UPARAM(ref) FAGX_ConstraintScrewController& Controller, float SpookDamping)
	{
		Controller.SetSpookDamping(static_cast<double>(SpookDamping));
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetSpookDamping(UPARAM(ref) const FAGX_ConstraintScrewController& Controller)
	{
		return static_cast<float>(Controller.GetSpookDamping());
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetForceRange(
		UPARAM(ref) FAGX_ConstraintScrewController& Controller, float MinForce, float MaxForce)
	{
		Controller.SetForceRange(MinForce, MaxForce);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForceRangeMin(UPARAM(ref) const FAGX_ConstraintScrewController& Controller)
	{
		return Controller.GetForceRange().Min;
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForceRangeMax(UPARAM(ref) const FAGX_ConstraintScrewController& Controller)
	{
		return Controller.GetForceRange().Max;
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForce(UPARAM(ref) FAGX_ConstraintScrewController& ControllerRef)
	{
		return static_cast<float>(ControllerRef.GetForce());
	}

	//~ End AGX_ConstraintController Blueprint Library interface.
};
