// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_NativeOwner.h"
#include "Sensors/IMUBarrier.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"

#include "AGX_IMUSensorComponent.generated.h"

/**
 * 3D Inertial Measurement Unit (IMU) Sensor Component, supporting sub-sensors: Accelerometer,
 * Gyroscope and Magnetometer which can be individually enabled or disabled. All three sub-sensor
 * types have a number of features such as signal noise, zero offset bias among others that can be
 * configured to yield realistic sensor data.
 *
 * Note that to use the IMU Sensor Component, it must be registered with an AGX Sensor Environment
 * Actor.
 */
UCLASS(ClassGroup = "AGX_Sensor", Category = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_IMUSensorComponent : public USceneComponent, public IAGX_NativeOwner
{
	GENERATED_BODY()

public:
	UAGX_IMUSensorComponent();

	/**
	 * Enable or disable this IMU Sensor Component.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX IMU")
	bool bEnabled {true};

	UFUNCTION(BlueprintCallable, Category = "AGX IMU")
	void SetEnabled(bool InEnabled);

	UFUNCTION(BlueprintCallable, Category = "AGX IMU")
	bool GetEnabled() const;

	/**
	 * Enable or disable Accelerometer for this IMU.
	 * The Accelerometer measures linear acceleration along the x, y, and z axis [cm/s^2].
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX IMU", Meta = (ExposeOnSpawn))
	bool bUseAccelerometer {true};

	/**
	 * Enable or disable Gyroscope for this IMU.
	 * The Gyroscope measures angular velocity around the x, y, and z axis [deg/s].
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX IMU", Meta = (ExposeOnSpawn))
	bool bUseGyroscope {true};

	/**
	 * Enable or disable Magnetometer for this IMU.
	 * The Magnetometer measures the magnetic field vector in Tesla [T].
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX IMU", Meta = (ExposeOnSpawn))
	bool bUseMagnetometer {false};

	/**
	 * Get the latest Accelerometer data from this IMU Sensor.
	 * This funcion should only be called during Play and if this IMU Sensor has an Accelerometer.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Accelerometer")
	FVector GetAcclerometerData() const;

	void UpdateNativeTransform();

	FIMUBarrier* GetOrCreateNative();
	FIMUBarrier* GetNative();
	const FIMUBarrier* GetNative() const;

	// ~Begin AGX NativeOwner interface.
	virtual bool HasNative() const override;
	virtual uint64 GetNativeAddress() const override;
	virtual void SetNativeAddress(uint64 NativeAddress) override;
	// ~/End IAGX_NativeOwner interface.

	//~ Begin UActorComponent Interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual TStructOnScope<FActorComponentInstanceData> GetComponentInstanceData() const override;
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
	//~ End UActorComponent Interface

	//~ Begin UObject interface.
#if WITH_EDITOR
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	virtual void PostInitProperties() override;
#endif
	//~ End UObject interface.

private:
	void UpdateNativeProperties();
	void CreateNative();

#if WITH_EDITOR
	void InitPropertyDispatcher();
#endif

	FIMUBarrier NativeBarrier;
};
