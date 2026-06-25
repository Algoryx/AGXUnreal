// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "OpenPLX/OpenPLXIMUOutputView.h"

// Unreal Engine includes.
#include "Kismet/BlueprintFunctionLibrary.h"

#include "OpenPLX_IMUOutputViewFunctionLibrary.generated.h"

UCLASS()
class AGXUNREAL_API UOpenPLX_IMUOutputView : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/// Returns true if the view references valid native IMU output data.
	UFUNCTION(BlueprintPure, Category = "OpenPLX IMU Output View")
	static bool HasNative(const FOpenPLXIMUOutputView& View)
	{
		return View.HasNative();
	}

	/// Return the number of samples in this IMU output view.
	UFUNCTION(BlueprintPure, Category = "OpenPLX IMU Output View")
	static int32 GetNumSamples(const FOpenPLXIMUOutputView& View)
	{
		return View.GetNumSamples();
	}

	/// Returns true if this view contains accelerometer samples.
	UFUNCTION(BlueprintPure, Category = "OpenPLX IMU Output View")
	static bool HasAccelerometer(const FOpenPLXIMUOutputView& View)
	{
		return View.HasAccelerometer();
	}

	/// Returns true if this view contains gyroscope samples.
	UFUNCTION(BlueprintPure, Category = "OpenPLX IMU Output View")
	static bool HasGyroscope(const FOpenPLXIMUOutputView& View)
	{
		return View.HasGyroscope();
	}

	/// Returns true if this view contains magnetometer samples.
	UFUNCTION(BlueprintPure, Category = "OpenPLX IMU Output View")
	static bool HasMagnetometer(const FOpenPLXIMUOutputView& View)
	{
		return View.HasMagnetometer();
	}

	/// Get accelerometer data in IMU-local Unreal coordinates [cm/s^2].
	UFUNCTION(BlueprintCallable, Category = "OpenPLX IMU Output View")
	static bool GetAccelerometerDataLocal(
		UPARAM(Ref) FOpenPLXIMUOutputView& View, FVector& OutAccelerometerData)
	{
		return View.GetAccelerometerData(OutAccelerometerData);
	}

	/// Get accelerometer data in world Unreal coordinates [cm/s^2].
	UFUNCTION(BlueprintCallable, Category = "OpenPLX IMU Output View")
	static bool GetAccelerometerDataWorld(
		UPARAM(Ref) FOpenPLXIMUOutputView& View, const FTransform& IMUTransform,
		FVector& OutAccelerometerData);

	/// Get gyroscope data in IMU-local Unreal coordinates [deg/s].
	UFUNCTION(BlueprintCallable, Category = "OpenPLX IMU Output View")
	static bool GetGyroscopeDataLocal(
		UPARAM(Ref) FOpenPLXIMUOutputView& View, FVector& OutGyroscopeData)
	{
		return View.GetGyroscopeData(OutGyroscopeData);
	}

	/// Get gyroscope data in world Unreal coordinates [deg/s].
	UFUNCTION(BlueprintCallable, Category = "OpenPLX IMU Output View")
	static bool GetGyroscopeDataWorld(
		UPARAM(Ref) FOpenPLXIMUOutputView& View, const FTransform& IMUTransform,
		FVector& OutGyroscopeData);

	/// Get magnetometer data in IMU-local Unreal coordinates [T].
	UFUNCTION(BlueprintCallable, Category = "OpenPLX IMU Output View")
	static bool GetMagnetometerDataLocal(
		UPARAM(Ref) FOpenPLXIMUOutputView& View, FVector& OutMagnetometerData)
	{
		return View.GetMagnetometerData(OutMagnetometerData);
	}

	/// Get magnetometer data in world Unreal coordinates [T].
	UFUNCTION(BlueprintCallable, Category = "OpenPLX IMU Output View")
	static bool GetMagnetometerDataWorld(
		UPARAM(Ref) FOpenPLXIMUOutputView& View, const FTransform& IMUTransform,
		FVector& OutMagnetometerData);

	/**
	 * Copy the underlying IMU output data into memory owned by this view.
	 * A newly received IMU output view references memory owned by the OpenPLX Control Interface
	 * and is only valid until another read reuses that buffer. Call this before storing the view
	 * for later use. This copies the complete IMU output buffer.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX IMU Output View")
	static bool MakePersistant(UPARAM(Ref) FOpenPLXIMUOutputView& View)
	{
		return View.MakePersistant();
	}
};
