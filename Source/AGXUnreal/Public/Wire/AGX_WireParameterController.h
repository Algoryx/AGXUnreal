// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Wire/WireParameterControllerBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_WireParameterController.generated.h"

class FWireParameterControllerBarrier;

USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_WireParameterController
{
	GENERATED_BODY()

	/**
	 * This value should be related the size of objects the wire is interacting with, to avoid
	 * tunneling [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "Wire Parameter Controller")
	double MaximumContactMovementOneTimestep {10.0};
	void SetMaximumContactMovementOneTimestep(double MaxMovement);
	double GetMaximumContactMovementOneTimestep() const;

	/**
	 * Set the minimum distance allowed between nodes [cm].
	 *
	 * I.e., a lumped element node will NOT be created closer than this distance from routed nodes.
	 */
	UPROPERTY(EditAnywhere, Category = "Wire Parameter Controller")
	double MinimumDistanceBetweenNodes {5.0};
	void SetMinimumDistanceBetweenNodes(double MinDistance);
	double GetMinimumDistanceBetweenNodes() const;

	/**
	 * Set the wire sphere shape radius multiplier.
	 *
	 * It's convenient to have larger geometry spheres for the lumped elements in a wire. It looks
	 * more natural and it helps the contact handling in the wires.
	 */
	UPROPERTY(EditAnywhere, Category = "Wire Parameter Controller")
	double RadiusMultiplier {1.001};
	void SetRadiusMultiplier(double Multiplier);
	double GetRadiusMultiplier() const;
	/** Only available when there is an underlying native AGX Dynamics object. */
	double GetScaledRadiusMultiplier(double WireRadius) const;

	/**
	 * The scale constant controls the insert/remove of lumped nodes in a wire.
	 *
	 * The parameter has an analytical value derived given the Nyquist frequency. The probability to
	 * have more lumped nodes in the wire increases with this scale constant.
	 */
	UPROPERTY(EditAnywhere, Category = "Wire Parameter Controller")
	double ScaleConstant {0.35};
	void SetScaleConstant(double InScaleConstant);
	double GetScaleConstant() const;

	/**
	 * A multiplier for tension scaling when deciding if a constraint could be replaced with a
	 * force.
	 */
	UPROPERTY(EditAnywhere, Category = "Wire Parameter Controller")
	double SplitTensionMultiplier {2.0};
	void SetSplitTensionMultiplier(double Multiplier);
	double GetSplitTensionMultiplier() const;

	/**
	 * The fraction of stop node reference distance that defines the range end of the prismatic
	 * constraint used by most winches.
	 *
	 * Default: 0.05 which means 5 cm if stop node reference distance is 100 cm.
	 */
	UPROPERTY(EditAnywhere, Category = "Wire Parameter Controller")
	double StopNodeLumpMinDistanceFraction {0.05};
	void SetStopNodeLumpMinDistanceFraction(double Fraction);
	double GetStopNodeLumpMinDistanceFraction() const;

	/**
	 * The distance between the lump node and the stop node for winches [cm].
	 *
	 * This also defines the theoretic maximum speed, in fact, in practice the speed can be higher
	 * but the behavior can be strange. Default: 1 m, equivalent to winch speed of 60 m/s in 60 Hz.
	 */
	UPROPERTY(EditAnywhere, Category = "Wire Parameter Controller")
	double StopNodeReferenceDistance {100};
	void SetStopNodeReferenceDistance(double Distance);
	double GetStopNodeReferenceDistance() const;

	/**
	 * A scale for the damping of the dynamics solver for ShapeContactNodes.
	 *
	 * This will scale the damping of the damping found in the material parameters for an
	 * interaction of a wire/other contact.
	 */
	UPROPERTY(EditAnywhere, Category = "Wire Parameter Controller")
	double WireContactDynamicsSolverDampingScale {2};
	void SetWireContactDynamicsSolverDampingScale(double Scale);
	double GetWireContactDynamicsSolverDampingScale() const;

	void SetBarrier(const FWireParameterControllerBarrier& InBarrier);
	void ClearBarrier();
	void CopyFrom(const FWireParameterControllerBarrier& Barrier);
	void WritePropertiesToNative();
	bool HasNative() const;

private:
	FWireParameterControllerBarrier NativeBarrier;
};

UCLASS()
class AGXUNREAL_API UAGX_WireParameterController_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/**
	 * This value should be related the size of objects the wire is interacting with, to avoid
	 * tunneling [cm].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Parameter Controller")
	static void SetMaximumContactMovementOneTimestep(
		UPARAM(Ref) FAGX_WireParameterController& Controller, double MaxMovement)
	{
		Controller.SetMaximumContactMovementOneTimestep(MaxMovement);
	}

	/**
	 * This value should be related the size of objects the wire is interacting with, to avoid
	 * tunneling [cm].
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Wire Parameter Controller")
	static double GetMaximumContactMovementOneTimeStep(UPARAM(Ref)
														   FAGX_WireParameterController& Controller)
	{
		return Controller.GetMaximumContactMovementOneTimestep();
	}

	/**
	 * Set the minimum distance allowed between nodes [cm].
	 *
	 * I.e., a lumped element node will NOT be created closer than this distance from routed nodes.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Parameter Controller")
	static void SetMinimumDistanceBetweenNodes(
		UPARAM(Ref) FAGX_WireParameterController& Controller, double MinDistance)
	{
		Controller.SetMinimumDistanceBetweenNodes(MinDistance);
	}

	/**
	 * Set the minimum distance allowed between nodes [cm].
	 *
	 * I.e., a lumped element node will NOT be created closer than this distance from routed nodes.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Wire Parameter Controller")
	static double GetMinimumDistanceBetweenNodes(UPARAM(Ref)
													 FAGX_WireParameterController& Controller)
	{
		return Controller.GetMinimumDistanceBetweenNodes();
	}

	/**
	 * Set the wire sphere shape radius multiplier.
	 *
	 * It's convenient to have larger geometry spheres for the lumped elements in a wire. It looks
	 * more natural and it helps the contact handling in the wires.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Parameter Controller")
	static void SetRadiusMultiplier(
		UPARAM(Ref) FAGX_WireParameterController& Controller, double RadiusMultiplier)
	{
		Controller.SetRadiusMultiplier(RadiusMultiplier);
	}

	/**
	 * Get the wire sphere shape radius multiplier.
	 *
	 * It's convenient to have larger geometry spheres for the lumped elements in a wire. It
	 * looks more natural and it helps the contact handling in the wires.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Wire Parameter Controller")
	static double GetRadiusMultiplier(UPARAM(Ref) FAGX_WireParameterController& Controller)
	{
		return Controller.GetRadiusMultiplier();
	}

	/**
	 *
	 * @param Controller Wire Parameter Controller to read from.
	 * @param WireRadius The radius of the wire [cm].
	 * @return Scaled wire radius parameter.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Wire Parameter Controller")
	static double GetScaledRadiusMultiplier(
		UPARAM(Ref) FAGX_WireParameterController& Controller, double WireRadius)
	{
		return Controller.GetScaledRadiusMultiplier(WireRadius);
	}

	/**
	 * The scale constant controls the insert/remove of lumped nodes in a wire.
	 *
	 * The parameter has an analytical value derived given the Nyquist frequency. The
	 * probability to have more lumped nodes in the wire increases with this scale constant.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Parameter Controller")
	static void SetScaleConstant(
		UPARAM(Ref) FAGX_WireParameterController& Controller, double ScaleConstant)
	{
		Controller.SetScaleConstant(ScaleConstant);
	}

	/**
	 * The scale constant controls the insert/remove of lumped nodes in a wire.
	 *
	 * The parameter has an analytical value derived given the Nyquist frequency. The
	 * probability to have more lumped nodes in the wire increases with this scale constant.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Wire Parameter Controller")
	static double GetScaleConstant(UPARAM(Ref) FAGX_WireParameterController& Controller)
	{
		return Controller.GetScaleConstant();
	}

	/**
	 * A multiplier for tension scaling when deciding if a constraint could be replaced with a
	 * force.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Parameter Controller")
	static void SetSplitTensionMultiplier(
		UPARAM(Ref) FAGX_WireParameterController& Controller, double Multiplier)
	{
		Controller.SetSplitTensionMultiplier(Multiplier);
	}

	/**
	 * A multiplier for tension scaling when deciding if a constraint could be replaced with a
	 * force.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Wire Parameter Controller")
	static double GetSplitTensionMultiplier(UPARAM(Ref) FAGX_WireParameterController& Controller)
	{
		return Controller.GetSplitTensionMultiplier();
	}

	/**
	 * The fraction of stop node reference distance that defines the range end of the prismatic
	 * constraint used by most winches.
	 *
	 * Default: 0.05 which means 5 cm if stop node reference distance is 100 cm.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Parameter Controller")
	static void SetStopNodeLumpMinDistanceFraction(
		UPARAM(Ref) FAGX_WireParameterController& Controller, double Fraction)
	{
		Controller.SetStopNodeLumpMinDistanceFraction(Fraction);
	}

	/**
	 * The fraction of stop node reference distance that defines the range end of the prismatic
	 * constraint used by most winches.
	 *
	 * Default: 0.05 which means 5 cm if stop node reference distance is 100 cm.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Wire Parameter Controller")
	static double GetStopNodeLumpMinDistanceFraction(UPARAM(Ref)
														 FAGX_WireParameterController& Controller)
	{
		return Controller.GetStopNodeLumpMinDistanceFraction();
	}

	/**
	 * The distance between the lump node and the stop node for winches [cm].
	 *
	 * This also defines the theoretic maximum speed, in fact, in practice the speed can be higher
	 * but the behavior can be strange. Default: 1 m, equivalent to winch speed of 60 m/s in 60 Hz.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Parameter Controller")
	static void SetStopNodeReferenceDistance(
		UPARAM(Ref) FAGX_WireParameterController& Controller, double Distance)
	{
		Controller.SetStopNodeReferenceDistance(Distance);
	}

	/**
	 * The distance between the lump node and the stop node for winches [cm].
	 *
	 * This also defines the theoretic maximum speed, in fact, in practice the speed can be higher
	 * but the behavior can be strange. Default: 1 m, equivalent to winch speed of 60 m/s in 60 Hz.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Wire Parameter Controller")
	static double GetStopNodeReferenceDistance(UPARAM(Ref) FAGX_WireParameterController& Controller)
	{
		return Controller.GetStopNodeReferenceDistance();
	}

	/**
	 * A scale for the damping of the dynamics solver for ShapeContactNodes.
	 *
	 * This will scale the damping of the damping found in the material parameters for an interaction of a wire/other contact.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Wire Parameter Controller")
	static void SetWireContactDynamicsSolverDampingScale(UPARAM(Ref) FAGX_WireParameterController& Controller, double Scale)
	{
		Controller.SetWireContactDynamicsSolverDampingScale(Scale);
	}

	/**
	 * A scale for the damping of the dynamics solver for ShapeContactNodes.
	 *
	 * This will scale the damping of the damping found in the material parameters for an interaction of a wire/other contact.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Wire Parameter Controller")
	static double GetWireContactDynamicsSolverDampingScale(UPARAM(Ref) FAGX_WireParameterController& Controller)
	{
		return  Controller.GetWireContactDynamicsSolverDampingScale();
	}
};
