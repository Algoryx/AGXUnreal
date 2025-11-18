// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_NativeOwner.h"
#include "Vehicle/AGX_SteeringParameters.h"
#include "Vehicle/AGX_WheelJointReference.h"
#include "Vehicle/SteeringBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "AGX_SteeringComponent.generated.h"

struct FAGX_ImportContext;

/**
 * The Steering Component provides a steering mechanism designed to align wheels simultaneously to
 * minimize slip, making them move along the tangents of concentric circles determined by the
 * distances between the wheels and the distance between front and rear.
 *
 * It provides:
 *  - Ackermann mechanism with direct steering input on one of the wheels.
 *  - Bell-crank (central-lever) linkage, used when the steering column is centered.
 *  - Rack and pinion mechanism, commonly used in passenger cars.
 *  - Davis mechanism, which provides ideal steering geometry, but is not used in practice.
 */
UCLASS(ClassGroup = "AGX_Vehicle", Category = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_SteeringComponent : public UActorComponent, public IAGX_NativeOwner
{
	GENERATED_BODY()

public:
	/*
	 * Determines whether this Steering Component is enabled (active) or not.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Steering")
	bool bEnabled {true};

	UFUNCTION(BlueprintCallable, Category = "AGX Steering")
	void SetEnabled(bool InEnabled);

	UFUNCTION(BlueprintCallable, Category = "AGX Steering")
	bool IsEnabled() const;

	/**
	 * Reference to the Wheel Joint on the left side of the vehicle that participates in steering.
	 * This Wheel Joint will have its steering angle controlled according to the configured Steering
	 * Type.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Steering", Meta = (ExposeOnSpawn))
	FAGX_WheelJointReference LeftWheelJoint;

	/**
	 * Reference to the Wheel Joint on the right side of the vehicle that participates in steering.
	 * This Wheel Joint will have its steering angle controlled according to the configured Steering
	 * Type.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Steering", Meta = (ExposeOnSpawn))
	FAGX_WheelJointReference RightWheelJoint;

	/**
	* The Steering Parameters used to configure the current steering type.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Steering", Meta = (ExposeOnSpawn))
	UAGX_SteeringParameters* SteeringParameters;

	/*
	 * The import Guid of this Component. Only used by the AGX Dynamics for Unreal import system.
	 * Should never be assigned manually.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Dynamics Import Guid")
	FGuid ImportGuid;

	/*
	 * The import name of this Component. Only used by the AGX Dynamics for Unreal import system.
	 * Should never be assigned manually.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Dynamics Import Name")
	FString ImportName;

	/**
	 * Set the current steering angle [deg].
	 * This function is only valid during runtime and will do nothing otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Steering")
	void SetSteeringAngle(double Angle);

	/**
	 * Get the current steering angle [deg].
	 * This function is only valid during runtime and will return 0.0 otherwise.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Steering")
	double GetSteeringAngle() const;

	void CopyFrom(const FSteeringBarrier& Barrier, FAGX_ImportContext* Context);

	FSteeringBarrier* GetNative();
	const FSteeringBarrier* GetNative() const;

	//~ Begin IAGX_NativeObject interface.
	virtual bool HasNative() const override;
	virtual uint64 GetNativeAddress() const override;
	virtual void SetNativeAddress(uint64 NativeAddress) override;
	//~ End IAGX_NativeObject interface.

	//~ Begin UActorComponent interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual TStructOnScope<FActorComponentInstanceData> GetComponentInstanceData() const override;
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
	//~ End UActorComponent interface

	// ~Begin UObject interface.
#if WITH_EDITOR
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
#endif
	virtual void OnRegister() override;
	// ~End UObject interface.


private:
#if WITH_EDITOR
	void InitPropertyDispatcher();
#endif
	void CreateNative();
	bool GetEnabled() const; // To be able to use AGX_COMPONENT_DEFAULT_DISPATCHER_BOOL.

	FSteeringBarrier NativeBarrier;
};
