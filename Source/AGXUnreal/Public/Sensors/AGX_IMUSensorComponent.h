// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_NativeOwner.h"
#include "AGX_RealInterval.h"
#include "AGX_RigidBodyReference.h"
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

	//
	// Accelerometer
	//

	/**
	 * Detectable range of acceleration for the accelerometer used by this IMU Sensor [cm/s^2].
	 * Acceleration output data will be clamped within this range.
	 * Applies equally to all axes.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX IMU|AGX Accelerometer",
		Meta = (EditCondition = "bUseAccelerometer"))
	FAGX_RealInterval AccelerometerRange {
		std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max()};

	/**
	 * Set the detectable range of acceleration for the accelerometer used by this IMU Sensor
	 * [cm/s^2].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Accelerometer")
	void SetAccelerometerRange(FAGX_RealInterval Range);

	/**
	 * Get the detectable range of acceleration for the accelerometer used by this IMU Sensor
	 * [cm/s^2].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Accelerometer")
	FAGX_RealInterval GetAccelerometerRange() const;

	/**
	 * The Accelerometer cross axis sensitivity specifies how much a measured acceleration in one
	 * axis bleeds over into the signal of another axis. I.e. this determines how much a measurement
	 * in the X axis effect the other axes. Valid range of each element is [0.0 - 1.0].
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX IMU|AGX Accelerometer",
		Meta = (EditCondition = "bUseAccelerometer", ClampMin = "0.0", ClampMax = "1.0"))
	FVector AccelerometerCrossAxisSensitivityX {1.0, 0.0, 0.0};

	/**
	 * Set the Accelerometer cross axis sensitivity for the X axis.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Accelerometer")
	void SetAccelerometerCrossAxisSensitivityX(FVector X);

	/**
	 * Get the Accelerometer cross axis sensitivity for the X axis.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Accelerometer")
	FVector GetAccelerometerCrossAxisSensitivityX();

	/**
	 * The Accelerometer cross axis sensitivity specifies how much a measured acceleration in one
	 * axis bleeds over into the signal of another axis. I.e. this determines how much a measurement
	 * in the Y axis effect the other axes. Valid range of each element is [0.0 - 1.0].
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX IMU|AGX Accelerometer",
		Meta = (EditCondition = "bUseAccelerometer", ClampMin = "0.0", ClampMax = "1.0"))
	FVector AccelerometerCrossAxisSensitivityY {0.0, 1.0, 0.0};

	/**
	 * Set the Accelerometer cross axis sensitivity for the Y axis.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Accelerometer")
	void SetAccelerometerCrossAxisSensitivityY(FVector Y);

	/**
	 * Get the Accelerometer cross axis sensitivity for the Y axis.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Accelerometer")
	FVector GetAccelerometerCrossAxisSensitivityY();

	/**
	 * The Accelerometer cross axis sensitivity specifies how much a measured acceleration in one
	 * axis bleeds over into the signal of another axis. I.e. this determines how much a measurement
	 * in the Z axis effect the other axes. Valid range of each element is [0.0 - 1.0].
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX IMU|AGX Accelerometer",
		Meta = (EditCondition = "bUseAccelerometer", ClampMin = "0.0", ClampMax = "1.0"))
	FVector AccelerometerCrossAxisSensitivityZ {0.0, 0.0, 1.0};

	/**
	 * Set the Accelerometer cross axis sensitivity for the Z axis.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Accelerometer")
	void SetAccelerometerCrossAxisSensitivityZ(FVector Z);

	/**
	 * Get the Accelerometer cross axis sensitivity for the Z axis.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Accelerometer")
	FVector GetAccelerometerCrossAxisSensitivityZ();

	/**
	 * Set the Accelerometer cross axis sensitivity for all axes.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Accelerometer")
	void SetAccelerometerCrossAxisSensitivity(FVector X, FVector Y, FVector Z);

	/**
	 * Specifies the measurement bias in each of the three axes at zero external acceleration
	 * [cm/s^2].
	 * The bias will be active at any measured acceleration.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX IMU|AGX Accelerometer",
		Meta = (EditCondition = "bUseAccelerometer"))
	FVector AccelerometerZeroGBias {0.0};

	/**
	 * Set the Accelerometer zero g bias [cm/s^2].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Accelerometer")
	void SetAccelerometerZeroGBias(FVector Bias);

	/**
	 * Get the Accelerometer zero g bias [cm/s^2].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Accelerometer")
	FVector GetAccelerometerZeroGBias() const;

	/**
	 * Applied as a gaussian noise to the output data with the given noise Root Mean Square (RMS)
	 * value [cm/s^2].
	 * The noise RMS value is applied to each axis of the IMU individually.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX IMU|AGX Accelerometer",
		Meta = (EditCondition = "bUseAccelerometer"))
	FVector AccelerometerNoiseRMS {0.0};

	/**
	 * Set a gaussian noise RMS value that will be applied to the Accelerometer output data
	 * [cm/s^2].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Accelerometer")
	void SetAccelerometerNoiseRMS(FVector NoiseRMS);

	/**
	 * Get the gaussian noise RMS value [cm/s^2].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Accelerometer")
	FVector GetAccelerometerNoiseRMS() const;

	/**
	 * Specifies the spectral noise density applied to the Accelerometer output data, i.e. a
	 * gaussian noise that is frequency dependant [(measurement unit)/Hz].
	 * The noise value is applied to each axis of the IMU individually.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX IMU|AGX Accelerometer",
		Meta = (EditCondition = "bUseAccelerometer"))
	FVector AccelerometerSpectralNoiseDensity {0.0};

	/**
	 * Set a gaussian spectral noise density value that will be applied to the Accelerometer output
	 * data.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Accelerometer")
	void SetAccelerometerSpectralNoiseDensity(FVector Noise);

	/**
	 * Get the gaussian spectral noise density value.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Accelerometer")
	FVector GetAccelerometerSpectralNoiseDensity() const;

	/**
	 * Get the latest Accelerometer data from this IMU Sensor (expressed in the local IMU Sensor
	 * frame). This funcion should only be called during Play and if this IMU Sensor has an
	 * Accelerometer [cm/s^2].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Accelerometer")
	FVector GetAccelerometerDataLocal() const;

	/**
	 * Get the latest Accelerometer data from this IMU Sensor (expressed in the world frame).
	 * This funcion should only be called during Play and if this IMU Sensor has an
	 * Accelerometer [cm/s^2].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Accelerometer")
	FVector GetAccelerometerDataWorld() const;

	//
	// Gyroscope
	//

	/**
	 * Detectable range of angular rate for the Gyroscope used by this IMU Sensor [deg/s].
	 * Gyroscope output data will be clamped within this range.
	 * Applies equally to all axes.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX IMU|AGX Gyroscope",
		Meta = (EditCondition = "bUseGyroscope"))
	FAGX_RealInterval GyroscopeRange {
		std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max()};

	/**
	 * Set the detectable range of angular rate for the Gyroscope used by this IMU Sensor
	 * [deg/s].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Gyroscope")
	void SetGyroscopeRange(FAGX_RealInterval Range);

	/**
	 * Get the detectable range of angular rate for the Gyroscope used by this IMU Sensor
	 * [deg/s].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Gyroscope")
	FAGX_RealInterval GetGyroscopeRange() const;

	/**
	 * The Gyroscope cross axis sensitivity specifies how much a measured angular velocity in one
	 * axis bleeds over into the signal of another axis. I.e. this determines how much a measurement
	 * in the X axis effect the other axes. Valid range of each element is [0.0 - 1.0].
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX IMU|AGX Gyroscope",
		Meta = (EditCondition = "bUseGyroscope", ClampMin = "0.0", ClampMax = "1.0"))
	FVector GyroscopeCrossAxisSensitivityX {1.0, 0.0, 0.0};

	/**
	 * Set the Gyroscope cross axis sensitivity for the X axis.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Gyroscope")
	void SetGyroscopeCrossAxisSensitivityX(FVector X);

	/**
	 * Get the Gyroscope cross axis sensitivity for the X axis.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Gyroscope")
	FVector GetGyroscopeCrossAxisSensitivityX();

	/**
	 * The Gyroscope cross axis sensitivity specifies how much a measured angular velocity in one
	 * axis bleeds over into the signal of another axis. I.e. this determines how much a measurement
	 * in the Y axis effect the other axes. Valid range of each element is [0.0 - 1.0].
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX IMU|AGX Gyroscope",
		Meta = (EditCondition = "bUseGyroscope", ClampMin = "0.0", ClampMax = "1.0"))
	FVector GyroscopeCrossAxisSensitivityY {0.0, 1.0, 0.0};

	/**
	 * Set the Gyroscope cross axis sensitivity for the Y axis.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Gyroscope")
	void SetGyroscopeCrossAxisSensitivityY(FVector Y);

	/**
	 * Get the Gyroscope cross axis sensitivity for the Y axis.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Gyroscope")
	FVector GetGyroscopeCrossAxisSensitivityY();

	/**
	 * The Gyroscope cross axis sensitivity specifies how much a measured angular velocity in one
	 * axis bleeds over into the signal of another axis. I.e. this determines how much a measurement
	 * in the Z axis effect the other axes. Valid range of each element is [0.0 - 1.0].
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX IMU|AGX Gyroscope",
		Meta = (EditCondition = "bUseGyroscope", ClampMin = "0.0", ClampMax = "1.0"))
	FVector GyroscopeCrossAxisSensitivityZ {0.0, 0.0, 1.0};

	/**
	 * Set the Gyroscope cross axis sensitivity for the Z axis.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Gyroscope")
	void SetGyroscopeCrossAxisSensitivityZ(FVector Z);

	/**
	 * Get the Gyroscope cross axis sensitivity for the Z axis.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Gyroscope")
	FVector GetGyroscopeCrossAxisSensitivityZ();

	/**
	 * Set the Gyroscope cross axis sensitivity for all axes.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Gyroscope")
	void SetGyroscopeCrossAxisSensitivity(FVector X, FVector Y, FVector Z);

	/**
	 * Specifies the measurement bias in each of the three axes at zero angular rate
	 * [deg/s].
	 * The bias will be active at any measured angular rate.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX IMU|AGX Gyroscope",
		Meta = (EditCondition = "bUseGyroscope"))
	FVector GyroscopeZeroRateBias {0.0};

	/**
	 * Set the Gyroscope zero rate bias [deg/s].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Gyroscope")
	void SetGyroscopeZeroRateBias(FVector Bias);

	/**
	 * Get the Gyroscope zero rate bias [deg/s].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Gyroscope")
	FVector GetGyroscopeZeroRateBias() const;

	/**
	 * Applied as a gaussian noise to the output data with the given noise Root Mean Square (RMS)
	 * value [deg/s].
	 * The noise RMS value is applied to each axis of the IMU individually.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX IMU|AGX Gyroscope",
		Meta = (EditCondition = "bUseGyroscope"))
	FVector GyroscopeNoiseRMS {0.0};

	/**
	 * Set a gaussian noise RMS value that will be applied to the Gyroscope output data
	 * [deg/s].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Gyroscope")
	void SetGyroscopeNoiseRMS(FVector NoiseRMS);

	/**
	 * Get the gaussian noise RMS value [deg/s].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Gyroscope")
	FVector GetGyroscopeNoiseRMS() const;

	/**
	 * Specifies the spectral noise density applied to the Gyroscope output data, i.e. a
	 * gaussian noise that is frequency dependant [(measurement unit)/Hz].
	 * The noise value is applied to each axis of the IMU individually.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX IMU|AGX Gyroscope",
		Meta = (EditCondition = "bUseGyroscope"))
	FVector GyroscopeSpectralNoiseDensity {0.0};

	/**
	 * Set a gaussian spectral noise density value that will be applied to the Gyroscope output
	 * data.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Gyroscope")
	void SetGyroscopeSpectralNoiseDensity(FVector Noise);

	/**
	 * Get the gaussian spectral noise density value.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Gyroscope")
	FVector GetGyroscopeSpectralNoiseDensity() const;

	/**
	 * Lets accelerations along an axis perturb the output data for that axis, according to the
	 * specified multiplier.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX IMU|AGX Gyroscope",
		Meta = (EditCondition = "bUseGyroscope"))
	FVector GyroscopeLinearAccelerationEffects {0.0};

	/**
	 * Set the Gyroscope acceleration effects multiplier.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Gyroscope")
	void SetGyroscopeLinearAccelerationEffects(FVector AccelEffects);

	/**
	 * Get the Gyroscope acceleration effects multiplier.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Gyroscope")
	FVector GetGyroscopeLinearAccelerationEffects() const;

	/**
	 * Get the latest Gyroscope data from this IMU Sensor (expressed in the local IMU Sensor
	 * frame). This funcion should only be called during Play and if this IMU Sensor has a
	 * Gyroscope [deg/s].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Gyroscope")
	FVector GetGyroscopeDataLocal() const;

	/**
	 * Get the latest Gyroscope data from this IMU Sensor (expressed in the world frame).
	 * This funcion should only be called during Play and if this IMU Sensor has a
	 * Gyroscope [deg/s].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Gyroscope")
	FVector GetGyroscopeDataWorld() const;

	//
	// Magnetometer
	//

	/**
	 * Detectable range of magnetic flux for the Magnetometer used by this IMU Sensor [T].
	 * Magnetometer output data will be clamped within this range.
	 * Applies equally to all axes.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX IMU|AGX Magnetometer",
		Meta = (EditCondition = "bUseMagnetometer"))
	FAGX_RealInterval MagnetometerRange {
		std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max()};

	/**
	 * Set the detectable range of magnetic flux for the Magnetometer used by this IMU Sensor
	 * [T].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Magnetometer")
	void SetMagnetometerRange(FAGX_RealInterval Range);

	/**
	 * Get the detectable range of magnetic flux for the Magnetometer used by this IMU Sensor
	 * [T].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Magnetometer")
	FAGX_RealInterval GetMagnetometerRange() const;

	/**
	 * The Magnetometer cross axis sensitivity specifies how much a magnetic field applied to one
	 * axis bleeds over into the signal of another axis. I.e. this determines how much a measurement
	 * in the X axis effect the other axes. Valid range of each element is [0.0 - 1.0].
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX IMU|AGX Magnetometer",
		Meta = (EditCondition = "bUseMagnetometer", ClampMin = "0.0", ClampMax = "1.0"))
	FVector MagnetometerCrossAxisSensitivityX {1.0, 0.0, 0.0};

	/**
	 * Set the Magnetometer cross axis sensitivity for the X axis.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Magnetometer")
	void SetMagnetometerCrossAxisSensitivityX(FVector X);

	/**
	 * Get the Magnetometer cross axis sensitivity for the X axis.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Magnetometer")
	FVector GetMagnetometerCrossAxisSensitivityX();

	/**
	 * The Magnetometer cross axis sensitivity specifies how much a magnetic field applied to one
	 * axis bleeds over into the signal of another axis. I.e. this determines how much a measurement
	 * in the Y axis effect the other axes. Valid range of each element is [0.0 - 1.0].
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX IMU|AGX Magnetometer",
		Meta = (EditCondition = "bUseMagnetometer", ClampMin = "0.0", ClampMax = "1.0"))
	FVector MagnetometerCrossAxisSensitivityY {0.0, 1.0, 0.0};

	/**
	 * Set the Magnetometer cross axis sensitivity for the Y axis.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Magnetometer")
	void SetMagnetometerCrossAxisSensitivityY(FVector Y);

	/**
	 * Get the Magnetometer cross axis sensitivity for the Y axis.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Magnetometer")
	FVector GetMagnetometerCrossAxisSensitivityY();

	/**
	 * The Magnetometer cross axis sensitivity specifies how much a magnetic field applied to one
	 * axis bleeds over into the signal of another axis. I.e. this determines how much a measurement
	 * in the Z axis effect the other axes. Valid range of each element is [0.0 - 1.0].
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX IMU|AGX Magnetometer",
		Meta = (EditCondition = "bUseMagnetometer", ClampMin = "0.0", ClampMax = "1.0"))
	FVector MagnetometerCrossAxisSensitivityZ {0.0, 0.0, 1.0};

	/**
	 * Set the Magnetometer cross axis sensitivity for the Z axis.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Magnetometer")
	void SetMagnetometerCrossAxisSensitivityZ(FVector Z);

	/**
	 * Get the Magnetometer cross axis sensitivity for the Z axis.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Magnetometer")
	FVector GetMagnetometerCrossAxisSensitivityZ();

	/**
	 * Set the Magnetometer cross axis sensitivity for all axes.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Magnetometer")
	void SetMagnetometerCrossAxisSensitivity(FVector X, FVector Y, FVector Z);

	/**
	 * Specifies the measurement bias in each of the three axes at zero magnetic flux
	 * [T].
	 * The bias will be active at any measured magnetic flux.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX IMU|AGX Magnetometer",
		Meta = (EditCondition = "bUseMagnetometer"))
	FVector MagnetometerZeroFluxBias {0.0};

	/**
	 * Set the Magnetometer zero flux bias [T].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Magnetometer")
	void SetMagnetometerZeroFluxBias(FVector Bias);

	/**
	 * Get the Magnetometer zero flux bias [T].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Magnetometer")
	FVector GetMagnetometerZeroFluxBias() const;

	/**
	 * Applied as a gaussian noise to the output data with the given noise Root Mean Square (RMS)
	 * value [T].
	 * The noise RMS value is applied to each axis of the IMU individually.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX IMU|AGX Magnetometer",
		Meta = (EditCondition = "bUseMagnetometer"))
	FVector MagnetometerNoiseRMS {0.0};

	/**
	 * Set a gaussian noise RMS value that will be applied to the Magnetometer output data
	 * [T].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Magnetometer")
	void SetMagnetometerNoiseRMS(FVector NoiseRMS);

	/**
	 * Get the gaussian noise RMS value [T].
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Magnetometer")
	FVector GetMagnetometerNoiseRMS() const;

	/**
	 * Specifies the spectral noise density applied to the Magnetometer output data, i.e. a
	 * gaussian noise that is frequency dependant [(measurement unit)/Hz].
	 * The noise value is applied to each axis of the IMU individually.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX IMU|AGX Magnetometer",
		Meta = (EditCondition = "bUseMagnetometer"))
	FVector MagnetometerSpectralNoiseDensity {0.0};

	/**
	 * Set a gaussian spectral noise density value that will be applied to the Magnetometer output
	 * data.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Magnetometer")
	void SetMagnetometerSpectralNoiseDensity(FVector Noise);

	/**
	 * Get the gaussian spectral noise density value.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Magnetometer")
	FVector GetMagnetometerSpectralNoiseDensity() const;

	/**
	 * Get the latest Magnetometer data from this IMU Sensor (expressed in the local IMU Sensor
	 * frame). This funcion should only be called during Play and if this IMU Sensor has a
	 * Magnetometer [T].
	 * Note that the magnetic field can be set in the AGX Sensor Environment.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Magnetometer")
	FVector GetMagnetometerDataLocal() const;

	/**
	 * Get the latest Magnetometer data from this IMU Sensor (expressed in the world frame).
	 * This funcion should only be called during Play and if this IMU Sensor has a
	 * Magnetometer [T].
	 * Note that the magnetic field can be set in the AGX Sensor Environment.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU|AGX Magnetometer")
	FVector GetMagnetometerDataWorld() const;

	/**
	 * Set the position of the IMU Sensor, in world coordinates.
	 * This will update the local transform of the IMU Sensor in relation to the Rigid Body it is
	 * attached to.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU")
	void SetPosition(FVector Position);

	/**
	 * Get the position of the IMU Sensor, in world coordinates.
	 * This is the exact, ideal position based on the actual frame of the IMU Sensor, it is not
	 * based on the output data produced by any sub-sensor.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX IMU")
	FVector GetPosition() const;

	/**
	 * Set the rotation of the IMU Sensor, in world coordinates.
	 * This will update the local transform of the IMU Sensor in relation to the Rigid Body it is
	 * attached to.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX IMU")
	void SetRotation(FQuat Rotation);

	/**
	 * Get the rotation of the IMU Sensor, in world coordinates.
	 * This is the exact, ideal position based on the actual frame of the IMU Sensor, it is not
	 * based on the output data produced by any sub-sensor.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX IMU")
	FQuat GetRotation() const;

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
