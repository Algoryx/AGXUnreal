// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

#include "OpenPLXIMUOutputView.generated.h"

struct FOpenPLXIMUOutputViewRef;

/**
 * View into IMU output data received through OpenPLX.
 *
 * By default this struct references memory owned by the OpenPLX Control Interface. No IMU output
 * data is copied until data is requested. A newly received view is only valid until another
 * read operation reuses the underlying Control Interface buffer. Call MakePersistant before storing
 * the view for later use.
 */
USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FOpenPLXIMUOutputView
{
	GENERATED_BODY()

	FOpenPLXIMUOutputView();
	FOpenPLXIMUOutputView(std::shared_ptr<FOpenPLXIMUOutputViewRef> Native);

	bool HasNative() const;

	/// Return the number of samples in this IMU output view.
	int32 GetNumSamples() const;

	/// Returns true if this view contains accelerometer samples.
	bool HasAccelerometer() const;

	/// Returns true if this view contains gyroscope samples.
	bool HasGyroscope() const;

	/// Returns true if this view contains magnetometer samples.
	bool HasMagnetometer() const;

	/// Read accelerometer samples in Unreal coordinates [cm/s^2].
	bool ReadAccelerometer(TArray<FVector>& OutAccelerometer);

	/// Read gyroscope samples in Unreal coordinates [deg/s].
	bool ReadGyroscope(TArray<FVector>& OutGyroscope);

	/// Read magnetometer samples in Unreal coordinates [T].
	bool ReadMagnetometer(TArray<FVector>& OutMagnetometer);

	/**
	 * Copy the underlying IMU output data into memory owned by this view.
	 * A newly received IMU output view references memory owned by the OpenPLX Control Interface
	 * and is only valid until another read reuses that buffer. Call this before storing the view
	 * for later use. This copies the complete IMU output buffer.
	 */
	bool MakePersistant();

	FOpenPLXIMUOutputViewRef* GetNative();
	const FOpenPLXIMUOutputViewRef* GetNative() const;

private:
	std::shared_ptr<FOpenPLXIMUOutputViewRef> NativeRef;
};
