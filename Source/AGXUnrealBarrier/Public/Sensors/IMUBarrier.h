// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/SensorBarrier.h"

#include "IMUBarrier.generated.h"

struct FAGX_RealInterval;
struct FRigidBodyBarrier;

struct FIMUAllocationParameters
{
	bool bUseAccelerometer {false};
	bool bUseGyroscope {false};
	bool bUseMagnetometer {false};
	FTransform LocalTransform;
};

USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FIMUBarrier : public FSensorBarrier
{
	GENERATED_BODY()

	virtual ~FIMUBarrier() override = default;

	void AllocateNative(const FIMUAllocationParameters& Params, FRigidBodyBarrier& Body);

	/// Set the transform in world coordinate system.
	void SetTransform(const FTransform& Transform);

	/// Get the transform in world coordinate system.
	FTransform GetTransform() const;

	void SetPosition(FVector Position);
	FVector GetPosition() const;

	void SetRotation(FQuat Rotation);
	FQuat GetRotation() const;

	//
	// Accelerometer
	//

	void SetAccelerometerRange(FAGX_RealInterval Range);
	FAGX_RealInterval GetAccelerometerRange() const;

	void SetAccelerometerCrossAxisSensitivityX(FVector X);
	FVector GetAccelerometerCrossAxisSensitivityX() const;

	void SetAccelerometerCrossAxisSensitivityY(FVector Y);
	FVector GetAccelerometerCrossAxisSensitivityY() const;

	void SetAccelerometerCrossAxisSensitivityZ(FVector Z);
	FVector GetAccelerometerCrossAxisSensitivityZ() const;

	void SetAccelerometerZeroGBias(FVector Bias);
	FVector GetAccelerometerZeroGBias() const;

	void SetAccelerometerNoiseRMS(const FVector& Noise);
	FVector GetAccelerometerNoiseRMS() const;

	void SetAccelerometerSpectralNoiseDensity(const FVector& Noise);
	FVector GetAccelerometerSpectralNoiseDensity() const;

	/**
	 * Gets the latest Accelerometer output data from the IMU.
	 * The Accelerometer data is expressed in the IMU's frame.
	 * This data is only valid if this IMU was created with an Accelerometer.
	 */
	FVector GetAccelerometerData() const;

	//
	// Gyroscope
	//

	void SetGyroscopeRange(FAGX_RealInterval Range);
	FAGX_RealInterval GetGyroscopeRange() const;

	void SetGyroscopeCrossAxisSensitivityX(FVector X);
	FVector GetGyroscopeCrossAxisSensitivityX() const;

	void SetGyroscopeCrossAxisSensitivityY(FVector Y);
	FVector GetGyroscopeCrossAxisSensitivityY() const;

	void SetGyroscopeCrossAxisSensitivityZ(FVector Z);
	FVector GetGyroscopeCrossAxisSensitivityZ() const;

	void SetGyroscopeZeroRateBias(FVector Bias);
	FVector GetGyroscopeZeroRateBias() const;

	void SetGyroscopeNoiseRMS(const FVector& Noise);
	FVector GetGyroscopeNoiseRMS() const;

	void SetGyroscopeSpectralNoiseDensity(const FVector& Noise);
	FVector GetGyroscopeSpectralNoiseDensity() const;

	void SetGyroscopeLinearAccelerationEffects(
		const FVector& Effects); // No clean AGX getter exists.

	/**
	 * Gyroscope angular velocity in the IMU frame [deg/s]. Valid only if IMU has a Gyroscope.
	 */
	FVector GetGyroscopeData() const;

	//
	// Magnetometer
	//

	void SetMagnetometerRange(FAGX_RealInterval Range);
	FAGX_RealInterval GetMagnetometerRange() const;

	void SetMagnetometerCrossAxisSensitivityX(FVector X);
	FVector GetMagnetometerCrossAxisSensitivityX() const;

	void SetMagnetometerCrossAxisSensitivityY(FVector Y);
	FVector GetMagnetometerCrossAxisSensitivityY() const;

	void SetMagnetometerCrossAxisSensitivityZ(FVector Z);
	FVector GetMagnetometerCrossAxisSensitivityZ() const;

	void SetMagnetometerZeroFluxBias(FVector Bias);
	FVector GetMagnetometerZeroFluxBias() const;

	void SetMagnetometerNoiseRMS(const FVector& Noise);
	FVector GetMagnetometerNoiseRMS() const;

	void SetMagnetometerSpectralNoiseDensity(const FVector& Noise);
	FVector GetMagnetometerSpectralNoiseDensity() const;

	/**
	 * Magnetometer field vector in the IMU frame [T]. Valid only if IMU has a Magnetometer.
	 */
	FVector GetMagnetometerData() const;

	void MarkOutputAsRead();
};
