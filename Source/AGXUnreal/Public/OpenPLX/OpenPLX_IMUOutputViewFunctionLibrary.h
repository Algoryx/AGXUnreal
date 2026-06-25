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

	/// Read accelerometer samples in Unreal coordinates [cm/s^2].
	UFUNCTION(BlueprintCallable, Category = "OpenPLX IMU Output View")
	static bool ReadAccelerometer(
		UPARAM(Ref) FOpenPLXIMUOutputView& View, TArray<FVector>& OutAccelerometer)
	{
		return View.ReadAccelerometer(OutAccelerometer);
	}

	/// Read gyroscope samples in Unreal coordinates [deg/s].
	UFUNCTION(BlueprintCallable, Category = "OpenPLX IMU Output View")
	static bool ReadGyroscope(
		UPARAM(Ref) FOpenPLXIMUOutputView& View, TArray<FVector>& OutGyroscope)
	{
		return View.ReadGyroscope(OutGyroscope);
	}

	/// Read magnetometer samples in Unreal coordinates [T].
	UFUNCTION(BlueprintCallable, Category = "OpenPLX IMU Output View")
	static bool ReadMagnetometer(
		UPARAM(Ref) FOpenPLXIMUOutputView& View, TArray<FVector>& OutMagnetometer)
	{
		return View.ReadMagnetometer(OutMagnetometer);
	}

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
