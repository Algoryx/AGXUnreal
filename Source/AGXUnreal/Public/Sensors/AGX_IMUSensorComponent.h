// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_NativeOwner.h"
#include "AGX_RigidBodyReference.h"
#include "Sensors/IMUBarrier.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"

#include "AGX_IMUSensorComponent.generated.h"

struct FAGX_RealInterval;

/**
 * 3D Inertial Measurement Unit (IMU) Sensor Component, supporting sub-sensors: Accelerometer,
 * Gyroscope and Magnetometer which can be individually enabled or disabled. All three sub-sensor
 * types have a number of features such as signal noise, zero offset bias among others that can be
 * configured to yield realistic sensor data.
 *
 * To get the latest data from the sub-sensors, the GetAccelerometerData, GetGyroscopeData and
 * GetMagnetometer data can be called respectively.
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
	 * The Rigid Body this IMU Sensor is rigidly attached to.
	 * The relative transform this IMU Sensor has to this body at BeginPlay (or when registered) is
	 * preserved, keeping that offset fixed for the duration of the simulation.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX IMU")
	FAGX_RigidBodyReference RigidBody;

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
	 * Detectable range of acceleration for the accelerometer used by this IMU Sensor [cm/s^2].
	 * Acceleration output data will be clamped within this range.
	 * Applies equally to all axes.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Accelerometer",
		Meta = (EditCondition = "bUseAccelerometer"))
	FAGX_RealInterval AccelerometerRange {
		std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max()};

	/**
	 * Set the detectable range of acceleration for the accelerometer used by this IMU Sensor
	 * [cm/s^2].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Accelerometer")
	void SetAccelerometerRange(FAGX_RealInterval Range);

	/**
	 * Get the detectable range of acceleration for the accelerometer used by this IMU Sensor
	 * [cm/s^2].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Accelerometer")
	FAGX_RealInterval GetAccelerometerRange() const;

	/**
	 * The Accelerometer axis cross sensitivity specifies how much an acceleration applied to one
	 * axis bleeds over into the signal of another axis.
	 * Applies equally to all axes.
	 * Valid range is [0.0 - 1.0].
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Accelerometer",
		Meta = (EditCondition = "bUseAccelerometer", ClampMin = "0.0", ClampMax = "1.0"))
	double AccelerometerAxisCrossSensitivity {0.0};

	/**
	 * Set the Accelerometer axis cross sensitivity.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Accelerometer")
	void SetAccelerometerAxisCrossSensitivity(double Sensitivity);

	/**
	 * Get the Accelerometer axis cross sensitivity.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Accelerometer")
	double GetAccelerometerAxisCrossSensitivity() const;

	/**
	 * Specifies the measurement bias in each of the three axes at zero external acceleration
	 * [cm/s^2].
	 * The bias will be active at any measurable acceleration.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Accelerometer",
		Meta = (EditCondition = "bUseAccelerometer"))
	FVector AccelerometerZeroGBias {0.0};

	/**
	 * Set the Accelerometer zero g bias [cm/s^2].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Accelerometer")
	void SetAccelerometerZeroGBias(FVector Bias);

	/**
	 * Get the Accelerometer zero g bias [cm/s^2].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Accelerometer")
	FVector GetAccelerometerZeroGBias() const;

	/**
	 * Applied as a gaussian noise to the output data with the given noise Root Mean Square (RMS)
	 * value [cm/s^2].
	 * The noise RMS value is applied to each axis of the IMU individually.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Accelerometer",
		Meta = (EditCondition = "bUseAccelerometer"))
	FVector AccelerometerNoiseRMS {0.0};

	/**
	 * Set a gaussian noise RMS value that will be applied to the Accelerometer output data [cm/s^2].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Accelerometer")
	void SetAccelerometerNoiseRMS(FVector NoiseRMS);

	/**
	 * Get the gaussian noise RMS value [cm/s^2].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Accelerometer")
	FVector GetAccelerometerNoiseRMS() const;

	/**
	 * Specifies the spectral noise density applied to the Accelerometer output data, i.e. a
	 * gaussian noise that is frequency dependant [(measurement unit)/Hz].
	 * The noise value is applied to each axis of the IMU individually.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Accelerometer",
		Meta = (EditCondition = "bUseAccelerometer"))
	FVector AccelerometerSpectralNoiseDensity {0.0};

	/**
	 * Set a gaussian spectral noise density value that will be applied to the Accelerometer output data.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Accelerometer")
	void SetAccelerometerSpectralNoiseDensity(FVector Noise);

	/**
	 * Get the gaussian spectral noise density value.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Accelerometer")
	FVector GetAccelerometerSpectralNoiseDensity() const;

	/**
	 * Get the latest Accelerometer data from this IMU Sensor (expressed in the local IMU Sensor
	 * frame). This funcion should only be called during Play and if this IMU Sensor has an
	 * Accelerometer [cm/s^2].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Accelerometer")
	FVector GetAccelerometerDataLocal() const;

	/**
	 * Get the latest Accelerometer data from this IMU Sensor (expressed in the world frame).
	 * This funcion should only be called during Play and if this IMU Sensor has an
	 * Accelerometer [cm/s^2].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Accelerometer")
	FVector GetAccelerometerDataWorld() const;

	/**
	 * Get the latest Gyroscope data from this IMU Sensor (expressed in the local IMU Sensor
	 * frame). This funcion should only be called during Play and if this IMU Sensor has a
	 * Gyroscope [deg/s].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Gyroscope")
	FVector GetGyroscopeDataLocal() const;

	/**
	 * Get the latest Gyroscope data from this IMU Sensor (expressed in the world frame).
	 * This funcion should only be called during Play and if this IMU Sensor has a
	 * Gyroscope [deg/s].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Gyroscope")
	FVector GetGyroscopeDataWorld() const;

	/**
	 * Get the latest Magnetometer data from this IMU Sensor (expressed in the local IMU Sensor
	 * frame). This funcion should only be called during Play and if this IMU Sensor has a
	 * Magnetometer [T].
	 * Note that the magnetic field can be set in the AGX Sensor Environment.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Magnetometer")
	FVector GetMagnetometerDataLocal() const;

	/**
	 * Get the latest Magnetometer data from this IMU Sensor (expressed in the world frame).
	 * This funcion should only be called during Play and if this IMU Sensor has a
	 * Magnetometer [T].
	 * Note that the magnetic field can be set in the AGX Sensor Environment.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Magnetometer")
	FVector GetMagnetometerDataWorld() const;

	void UpdateTransformFromNative();

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
	virtual void OnRegister() override;
	//~ End UObject interface.

private:
	void UpdateNativeProperties();
	void CreateNative();

#if WITH_EDITOR
	void InitPropertyDispatcher();
#endif

	FIMUBarrier NativeBarrier;
};
