// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/IMUBarrier.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"

#include "AGX_IMUSensorComponent.generated.h"

/**
 * 3D Inertial Measurement Unit (IMU) Sensor Component, supporting sub-sensors: Accelerometer, Gyroscope and
 * Magnetometer which can be individually enabled or disabled.
 * All three sub-sensor types have a number of features such as signal noise, zero offset bias among
 * others that can be configured to yield realistic sensor data.
 */
UCLASS(ClassGroup = "AGX_Sensor", Category = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_IMUSensorComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UAGX_IMUSensorComponent();

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

	//~ Begin UActorComponent Interface
#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
	//~ End UActorComponent Interface

	FIMUBarrier NativeBarrier;
};
