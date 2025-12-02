// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_Constraint2DOFFreeDOF.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"
#include "InputCoreTypes.h"

#include "AGX_InputTargetSpeed.generated.h"

/**
 * Component that checks for particular keyboard presses and set a constraint
 * motor speed in response.
 *
 * Should only be added to AGX_Constraint1DOF actors.
 */
UCLASS(ClassGroup = (Custom), Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_InputTargetSpeed : public USceneComponent
{
	GENERATED_BODY()

public:
	UAGX_InputTargetSpeed();

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Speed Input",
		Meta = (Tooltip = "Key to trigger forward motion of the constraint."))
	FKey ForwardKey;

	/**
	 * The Constraint speed forward [cm/s] or [deg/s].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Speed Input",
		Meta = (Tooltip = "The constraint speed forward [cm/s] or [deg/s]."))
	float ForwardSpeed;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Speed Input",
		Meta = (Tooltip = "Key to trigger backward motion of the constraint."))
	FKey BackwardKey;

	/**
	 * The Constraint speed backward [cm/s] or [deg/s].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Speed Input",
		Meta = (Tooltip = "The constraint speed backward [cm/s] or [deg/s]."))
	float BackwardSpeed;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Speed Input",
		// Unreal Header Tool does not support line breaks in Property Specifier strings.
		// clang-format off
		Meta = (Tooltip = "If checked the constraint will move freely when no key is held. If unchecked the constraint will be stopped when no key is held."))
	// clang-format on
	bool bDisableOnRelease;

	UPROPERTY(
		EditAnywhere, Category = "AGX Constraint Speed Input",
		Meta = (EditCondition = "bMustSelectDOF"))
	EAGX_Constraint2DOFFreeDOF TargetDOF;

	// I want to make this a private property, but I don't know how. Setting
	// VisibleInstanceOnly as a workaround/hack for now.
	UPROPERTY(Category = "AGX Constraint Speed Input", VisibleInstanceOnly)
	bool bMustSelectDOF;

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(
		float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;

	void LogErrorMessageOnce(const TCHAR* Message);

private:
	bool bErrorMessageLogged = false;
};
