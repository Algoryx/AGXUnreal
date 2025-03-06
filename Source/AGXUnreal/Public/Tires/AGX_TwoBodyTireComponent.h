// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Tires/AGX_TireComponent.h"
#include "AGX_RigidBodyReference.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "AGX_TwoBodyTireComponent.generated.h"

class FTwoBodyTireBarrier;
struct FAGX_ImportContext;

/**
 * TwoBodyTire is a tire model that uses two Rigid Bodies, a tire Rigid Body and a hub Rigid Body.
 * The axis of rotation is always assumed to be around the y-axis of the final tire transform which
 * is defined by the tire Rigid Body's transform plus a local offset given by LocalLocation and
 * LocalRotation.
 */
UCLASS(Category = "AGX", ClassGroup = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_TwoBodyTireComponent : public UAGX_TireComponent
{
	GENERATED_BODY()

public:
	UAGX_TwoBodyTireComponent();
	virtual ~UAGX_TwoBodyTireComponent() = default;

	/**
	 * Outer radius of the tire [cm].
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Tire")
	float OuterRadius;

	/**
	 * Inner radius of the tire (and outer radius of hub) [cm].
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Tire")
	float InnerRadius;

	/**
	 * Reference to the Tire Rigid Body.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Tire")
	FAGX_RigidBodyReference TireRigidBody;

	/**
	 * Reference to the Hub Rigid Body.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Tire")
	FAGX_RigidBodyReference HubRigidBody;

	/**
	 * Tire relative location from Tire Rigid Body [cm].
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Tire")
	FVector LocalLocation;

	/**
	 * Tire relative rotation from Tire Rigid Body [deg].
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Tire")
	FRotator LocalRotation;

	/**
	 * Radial stiffness affects translation between the Hub Rigid Body and Tire Rigid Body
	 * orthogonal to tire rotation axis [N/m].
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Tire Dynamics")
	float RadialStiffness = 500000.f;

	/**
	 * Lateral stiffness affects translation between the Hub Rigid Body and Tire Rigid Body along
	 * axis of rotation [N/m].
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Tire Dynamics")
	float LateralStiffness = 50000000.f;

	/**
	 * Bending stiffness affects rotation between the Hub Rigid Body and Tire Rigid Body orthogonal
	 * to axis of rotation [Nm/rad].
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Tire Dynamics")
	float BendingStiffness = 250000.f;

	/**
	 * Torsional stiffness affects rotation between the Hub Rigid Body and Tire Rigid Body around
	 * axis of rotation [Nm/rad].
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Tire Dynamics")
	float TorsionalStiffness = 250000.f;

	/**
	 * Radial damping affects translation between the Hub Rigid Body and Tire Rigid Body orthogonal
	 * to tire rotation axis [Ns/m].
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Tire Dynamics")
	float RadialDamping = 16666.f;

	/**
	 * Lateral damping affects translation between the Hub Rigid Body and Tire Rigid Body along axis
	 * of rotation [Ns/m].
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Tire Dynamics")
	float LateralDamping = 1666666.f;

	/**
	 * Bending damping affects rotation between the Hub Rigid Body and Tire Rigid Body orthogonal to
	 * axis of rotation [Nms/rad].
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Tire Dynamics")
	float BendingDamping = 8333.f;

	/**
	 * Torsional damping affects rotation between the Hub Rigid Body and Tire Rigid Body around axis
	 * of rotation [Nms/rad].
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Tire Dynamics")
	float TorsionalDamping = 8333.f;

	/**
	 * Set the implicit friction multiplier in order to get different behavior for different
	 * friction directions (forwards, sideways). This is only necessary for implicit contact
	 * materials, since for explicit ones, this can be set directly at the contact material instead.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Tire Dynamics")
	FVector2D ImplicitFrictionMultiplier {1.f, 1.f};

	/*
	 * The import Guid of this Component. Only used by the AGX Dynamics for Unreal import system.
	 * Should never be assigned manually.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "AGX Dynamics Import Guid")
	FGuid ImportGuid;

	/**
	 * Get the Hub Rigid Body. Returns nullptr if it has not been set.
	 */
	UAGX_RigidBodyComponent* GetHubRigidBody() const;

	/**
	 * Get the Tire Rigid Body. Returns nullptr if it has not been set.
	 */
	UAGX_RigidBodyComponent* GetTireRigidBody() const;

	/**
	 * The final transform of the Tire model, defined by the transform of the Tire Rigid Body plus
	 * a local offset given by LocalLocation and LocalRotation.
	 */
	FTransform GetGlobalTireTransform() const;

	void CopyFrom(const FTwoBodyTireBarrier& Barrier, FAGX_ImportContext* Context);

	bool IsDefaultSubObjectOfTwoBodyTireActor() const;

	//~ Begin UActorComponent interface.
	virtual void PostInitProperties() override;
	virtual void PostLoad() override;
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	//~ End UActorComponent interface.

protected:
	virtual void AllocateNative() override;

	virtual void UpdateNativeProperties() override;

	void UpdateReferencesLocalScope();

private:
	FTwoBodyTireBarrier* CreateTwoBodyTireBarrier();
};
