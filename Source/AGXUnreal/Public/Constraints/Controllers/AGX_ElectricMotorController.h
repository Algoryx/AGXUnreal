// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_ConstraintController.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_ElectricMotorController.generated.h"

class FElectricMotorControllerBarrier;

/**
 * Electric motor controller for secondary constraints (usually on one of the
 * DOFs that has not been primarily constrained by the AGX Constraint). Disabled
 * by default.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ConstraintElectricMotorController : public FAGX_ConstraintController
{
	GENERATED_BODY()

	/**
	 * Available voltage or voltage drop across the terminals of this motor [V].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Electric Motor Controller",
		Meta = (EditCondition = "bEnable"))
	double Voltage {24.0};

	void SetVoltage(double InVoltage);

	double GetVoltage() const;

	/**
	 * Resistance in the armature circuit [Ohm].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Electric Motor Controller",
		Meta = (EditCondition = "bEnable"))
	double ArmatureResistance {1.0};

	void SetArmatureResistance(double InArmatureResistance);

	void SetArmatureRestistance(double InArmatureResistance);

	double GetArmatureResistance() const;

	/**
	 * Torque constant of this motor, in unit Torque Per Ampere [Nm/A].
	 * This value couples the torque out to current in.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Electric Motor Controller",
		Meta = (EditCondition = "bEnable"))
	double TorqueConstant {1.0};

	void SetTorqueConstant(double InTorqueConstant);

	double GetTorqueConstant() const;

public:
	FAGX_ConstraintElectricMotorController() = default;
	FAGX_ConstraintElectricMotorController(bool bRotational);

	void InitializeBarrier(TUniquePtr<FElectricMotorControllerBarrier> Barrier);
	void CopyFrom(
		const FElectricMotorControllerBarrier& Source,
		TArray<FAGX_ConstraintElectricMotorController*>& ArchetypeInstances,
		bool ForceOverwriteInstances);

private:
	virtual void UpdateNativePropertiesImpl() override;
};

/**
 * This class acts as an API that exposes functions of FAGX_ConstraintElectricMotorController in Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_ConstraintElectricMotorController_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()


	UFUNCTION(BlueprintCallable, Category = "AGX Electric Motor Controller")
	static void SetVoltage(UPARAM(ref) FAGX_ConstraintElectricMotorController& Controller, double InVoltage)
	{
		Controller.SetVoltage(InVoltage);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Electric Motor Controller")
	static double GetVoltage(UPARAM(ref) FAGX_ConstraintElectricMotorController& Controller)
	{
		return Controller.GetVoltage();
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Electric Motor Controller")
	static void SetArmatureRestistance(UPARAM(ref) FAGX_ConstraintElectricMotorController& Controller, double InArmatureResistance)
	{
		Controller.SetArmatureResistance(InArmatureResistance);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Electric Motor Controller")
	static double GetArmatureResistance(UPARAM(ref) FAGX_ConstraintElectricMotorController& Controller)
	{
		return Controller.GetArmatureResistance();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Electric Motor Controller")
	static void SetTorqueConstant(UPARAM(ref) FAGX_ConstraintElectricMotorController& Controller, double InTorqueConstant)
	{
		Controller.SetTorqueConstant(InTorqueConstant);
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Electric Motor Controller")
	static double GetTorqueConstant(UPARAM(ref) FAGX_ConstraintElectricMotorController& Controller)
	{
		return Controller.GetTorqueConstant();
	}

	//~ Begin AGX_ConstraintController Blueprint Library interface.
	// These are copy/pasted from FAGX_ConstraintController.h. See the comment in that file.

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static bool IsValid(UPARAM(ref) FAGX_ConstraintElectricMotorController& ControllerRef)
	{
		return ControllerRef.HasNative();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetEnable(
		UPARAM(ref) FAGX_ConstraintElectricMotorController& ControllerRef, bool Enable)
	{
		return ControllerRef.SetEnable(Enable);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static bool GetEnable(UPARAM(ref) FAGX_ConstraintElectricMotorController& ControllerRef)
	{
		return ControllerRef.GetEnable();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetCompliance(
		UPARAM(ref) FAGX_ConstraintElectricMotorController& Controller, float Compliance)
	{
		Controller.SetCompliance(static_cast<double>(Compliance));
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetCompliance(UPARAM(ref) const FAGX_ConstraintElectricMotorController& Controller)
	{
		return static_cast<float>(Controller.GetCompliance());
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetSpookDamping(
		UPARAM(ref) FAGX_ConstraintElectricMotorController& Controller, float SpookDamping)
	{
		Controller.SetSpookDamping(static_cast<double>(SpookDamping));
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetSpookDamping(UPARAM(ref)
									 const FAGX_ConstraintElectricMotorController& Controller)
	{
		return static_cast<float>(Controller.GetSpookDamping());
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetForceRange(
		UPARAM(ref) FAGX_ConstraintElectricMotorController& Controller, float MinForce,
		float MaxForce)
	{
		Controller.SetForceRange(MinForce, MaxForce);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForceRangeMin(UPARAM(ref)
									  const FAGX_ConstraintElectricMotorController& Controller)
	{
		return Controller.GetForceRange().Min;
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForceRangeMax(UPARAM(ref)
									  const FAGX_ConstraintElectricMotorController& Controller)
	{
		return Controller.GetForceRange().Max;
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForce(UPARAM(ref) FAGX_ConstraintElectricMotorController& ControllerRef)
	{
		return static_cast<float>(ControllerRef.GetForce());
	}

	//~ End AGX_ConstraintController Blueprint Library interface.
};
