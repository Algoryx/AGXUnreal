// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

class FRigidBodyBarrier;

struct FAGX_RealInterval;
struct FIMURef;

struct FIMUAllocationParameters
{
	bool bUseAccelerometer {false};
	bool bUseGyroscope {false};
	bool bUseMagnetometer {false};
	FTransform LocalTransform;
};

class AGXUNREALBARRIER_API FIMUBarrier
{
public:
	FIMUBarrier();
	FIMUBarrier(std::unique_ptr<FIMURef> Native);
	FIMUBarrier(FIMUBarrier&& Other);
	~FIMUBarrier();

	bool HasNative() const;
	void AllocateNative(const FIMUAllocationParameters& Params, FRigidBodyBarrier& Body);
	FIMURef* GetNative();
	const FIMURef* GetNative() const;
	uint64 GetNativeAddress() const;
	void SetNativeAddress(uint64 Address);
	void ReleaseNative();

	void SetEnabled(bool Enabled);
	bool GetEnabled() const;

	void SetTransform(const FTransform& Transform);
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

	void SetAccelerometerCrossAxisSensitivity(double Sensitivity); // No clean AGX getter exists.

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

	void SetGyroscopeCrossAxisSensitivity(double Sensitivity); // No clean AGX getter exists.

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

	void SetMagnetometerCrossAxisSensitivity(double Sensitivity); // No clean AGX getter exists.

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

	/**
	 * Increment the reference count of the AGX Dynamics object. This should always be paired with
	 * a call to DecrementRefCount, and the count should only be artificially incremented for a
	 * very well specified duration.
	 *
	 * One use-case is during a Blueprint Reconstruction, when the Unreal Engine objects are
	 * destroyed and then recreated. During this time the AGX Dynamics objects are retained and
	 * handed between the old and the new Unreal Engine objects through a Component Instance Data.
	 * This Component Instance Data instance is considered the owner of the AGX Dynamics object
	 * during this transition period and the reference count is therefore increment during its
	 * lifetime. We're lending out ownership of the AGX Dynamics object to the Component Instance
	 * Data instance for the duration of the Blueprint Reconstruction.
	 *
	 * These functions can be const even though they have observable side effects because the
	 * reference count is not a salient part of the AGX Dynamics objects, and they are thread-safe.
	 */
	void IncrementRefCount() const;
	void DecrementRefCount() const;

private:
	FIMUBarrier(const FIMUBarrier&) = delete;
	void operator=(const FIMUBarrier&) = delete;

private:
	std::unique_ptr<FIMURef> NativeRef;
};
