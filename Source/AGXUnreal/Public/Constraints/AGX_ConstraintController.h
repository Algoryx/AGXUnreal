// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"
#include "AGX_RealInterval.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Math/Interval.h"

#include "AGX_ConstraintController.generated.h"

class FConstraintControllerBarrier;

/**
 * Base struct for Constraint Controllers, also called Secondary Constraints. These are
 * sub-constraints that act on a degree of freedom that is unconstrained by the constraint. For
 * example, for a Hinge Constraint there is a free rotational axis around which the constrained
 * bodies can rotate. A Constraint Controller add functionality to this degree of freedom. There
 * are controllers that enforce a relative speed (Target Speed Controller), limiting it to a
 * particular range (Range Controller), outright locking (Lock Controller), and more.
 */
USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_ConstraintController
{
	GENERATED_BODY()

	/**
	 * Whether this Constraint Controller is active or not. A disabled Constraint Controller has
	 * no effect on the simulation.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Constraint Controller")
	bool bEnable;

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
	FAGX_Real Compliance;

	void SetCompliance(double InCompliance);

	double GetCompliance() const;

	/**
	 * Elasticity is defined as the inverse of compliance, so setting the compliance will modify
	 * the compliance [N/m] or [Nm/rad].
	 */
	void SetElasticity(double InElasticity);

	double GetElasticity() const;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Controller", Meta = (EditCondition = "bEnable"))
	double SpookDamping;

	void SetSpookDamping(double InSpookDamping);

	double GetSpookDamping() const;

	/**
	 * The minimum and maximum force or torque that the constraint controller can produce [N] or
	 * [Nm].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Controller", Meta = (EditCondition = "bEnable"))
	FAGX_RealInterval ForceRange;

	void SetForceRange(const FAGX_RealInterval& InForceRange);

	void SetForceRange(double MinForce, double MaxForce);

	FAGX_RealInterval GetForceRange() const;

	double GetForce();

	/**
	 * Handle serialization backwards compatibility. May be overridden by subclasses as long as they
	 * call Super::Serialize.
	 * @param Archive Archive to read from or write to.
	 */
	virtual void Serialize(FArchive& Archive);

protected:
	// Would like to have this const but Unreal provides default copy operations
	// that don't compile when USTRUCT structs contains constant members.
	// Perhaps there is a way to disable the default copy operations.
	/// Whether the controller is on a Rotational or Translational DOF.
	bool bRotational;

public:
	FAGX_ConstraintController();
	FAGX_ConstraintController(bool bInRotational);
	virtual ~FAGX_ConstraintController();

	// We must provide an assignment operator because Unreal must be able to copy structs and we
	// contain a non-copyable TUniquePtr to the underlying Barrier object. That object is not
	// copied.
	/**
	 * Copy the properties from Other into this. The underlying Barrier object will neither be
	 * copied nor shared.
	 * @param Other The object to copy parameters from.
	 * @return A reference to this.
	 */
	FAGX_ConstraintController& operator=(const FAGX_ConstraintController& Other);

	bool HasNative() const;
	FConstraintControllerBarrier* GetNative();
	const FConstraintControllerBarrier* GetNative() const;

	void UpdateNativeProperties();

protected:
	virtual void UpdateNativePropertiesImpl()
		PURE_VIRTUAL(FAGX_ConstraintController::UpdateNativePropertiesImpl, );

	/**
	 * Copy properties from the give AGX Dynamics constraint controller into this AGXUnreal
	 * constraint controller.
	 */
	void CopyFrom(const FConstraintControllerBarrier& Source);

	/**
	 * Handle to the AGX Dynamics instance.
	 *
	 * By pointer because there are many types of Constraint Controllers (Range, Speed, Lock, etc)
	 * and this may point to any of those subclasses of FConstraintControllerBarrier.
	 */
	TUniquePtr<FConstraintControllerBarrier> NativeBarrier;

private: // Deprecated functionality.
	/**
	 * The elasticity in a certain DOF. Measured in [N/m] for translational DOFs and [Nm/rad] for
	 * rotational DOFs.
	 * The elasticity measure the responsiveness/reactiveness of the Constraint Controller to
	 * violations in its constraint. A higher elasticity will cause a stronger force or torque to
	 * be created to restore from the violation. A too high elasticity will lead to instabilities in
	 * the simulation. It is the inverse of Compliance. It is measured in unit force or torque per
	 * unit violation, much like a spring constant, where the violation can be either a translation
	 * or a rotation.
	 *
	 * The unit is currently in Newton per meter or Newtonmeter per radian, the native AGX Dynamics
	 * units, but this may change to Newton per centimieter or Newtoncentimeter per degree, the
	 * Unreal Engine units, in the future.
	 */
	UPROPERTY()
	double Elasticity_DEPRECATED;
	// Deprecated because AGX Dynamics uses Compliance, which is the inverse of Elasticity. We
	// switched to Elasticity in AGX Dynamics for Unreal because Unreal Engine can't handle small
	// numbers and Compliance is typically small. Now that we have FAGX_Real to help with small
	// numbers we can switch back to Compliance again.
};

/**
 * This class acts as an API that exposes functions of FAGX_ConstraintController in Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_ConstraintController_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static bool IsValid(UPARAM(ref) FAGX_ConstraintController& ControllerRef)
	{
		return ControllerRef.HasNative();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetEnable(UPARAM(ref) FAGX_ConstraintController& ControllerRef, bool Enable)
	{
		return ControllerRef.SetEnable(Enable);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static bool GetEnable(UPARAM(ref) FAGX_ConstraintController& ControllerRef)
	{
		return ControllerRef.GetEnable();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetCompliance(UPARAM(ref) FAGX_ConstraintController& Controller, float Compliance)
	{
		Controller.SetCompliance(static_cast<double>(Compliance));
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetCompliance(UPARAM(ref) const FAGX_ConstraintController& Controller)
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
		UPARAM(ref) FAGX_ConstraintController& Controller, float SpookDamping)
	{
		Controller.SetSpookDamping(static_cast<double>(SpookDamping));
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetSpookDamping(UPARAM(ref) const FAGX_ConstraintController& Controller)
	{
		return static_cast<float>(Controller.GetSpookDamping());
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetForceRange(
		UPARAM(ref) FAGX_ConstraintController& Controller, float MinForce, float MaxForce)
	{
		Controller.SetForceRange(MinForce, MaxForce);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForceRangeMin(UPARAM(ref) const FAGX_ConstraintController& Controller)
	{
		return Controller.GetForceRange().Min;
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForceRangeMax(UPARAM(ref) const FAGX_ConstraintController& Controller)
	{
		return Controller.GetForceRange().Max;
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForce(UPARAM(ref) FAGX_ConstraintController& ControllerRef)
	{
		return static_cast<float>(ControllerRef.GetForce());
	}
};

// We have substructs of FAGX_ConstraintController which we also want to have the base struct
// Blueprint Library functions. Unfortunately, we have not found a way to automate this yet. The
// Blueprint Library declared above isn't callable on the subtypes and a #define containing
// all the function declarations and definitions doesn't work because UHT doesn't expand macros. So
// for now we're stuck with copy/paste. The following code block should be copy/pasted in each
// ConstraintController Blueprint Library class. Search/replace FAGX_TYPE and substitute the actual
// ConstraintController subtype.
/*
	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static bool IsValid(UPARAM(ref) FAGX_TYPE& ControllerRef)
	{
		return ControllerRef.HasNative();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetEnable(UPARAM(ref) FAGX_TYPE& ControllerRef, bool Enable)
	{
		return ControllerRef.SetEnable(Enable);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static bool GetEnable(UPARAM(ref) FAGX_TYPE& ControllerRef)
	{
		return ControllerRef.GetEnable();
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetCompliance(UPARAM(ref) FAGX_TYPE& Controller, float Compliance)
	{
		Controller.SetCompliance(static_cast<double>(Compliance));
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetCompliance(UPARAM(ref) const FAGX_TYPE& Controller)
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
	static void SetSpookDamping(UPARAM(ref) FAGX_TYPE& Controller, float SpookDamping)
	{
		Controller.SetSpookDamping(static_cast<double>(SpookDamping));
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetSpookDamping(UPARAM(ref) const FAGX_TYPE& Controller)
	{
		return static_cast<float>(Controller.GetSpookDamping());
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static void SetForceRange(
		UPARAM(ref) FAGX_TYPE& Controller, float MinForce, float MaxForce)
	{
		Controller.SetForceRange(MinForce, MaxForce);
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForceRangeMin(UPARAM(ref) const FAGX_TYPE& Controller)
	{
		return Controller.GetForceRange().Min;
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForceRangeMax(UPARAM(ref) const FAGX_TYPE& Controller)
	{
		return Controller.GetForceRange().Max;
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Constraint Controller")
	static float GetForce(UPARAM(ref) FAGX_TYPE& ControllerRef)
	{
		return static_cast<float>(ControllerRef.GetForce());
	}
*/
