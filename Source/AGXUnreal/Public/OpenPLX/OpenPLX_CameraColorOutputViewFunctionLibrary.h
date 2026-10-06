// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "OpenPLX/OpenPLXCameraColorOutputView.h"

// Unreal Engine includes.
#include "Kismet/BlueprintFunctionLibrary.h"

#include "OpenPLX_CameraColorOutputViewFunctionLibrary.generated.h"

/** Blueprint helpers for an OpenPLX Camera Color output view. */
UCLASS()
class AGXUNREAL_API UOpenPLX_CameraColorOutputView : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/// Returns true if the view references valid native Camera Color output data.
	UFUNCTION(BlueprintPure, Category = "OpenPLX Camera Color Output View")
	static bool HasNative(const FOpenPLXCameraColorOutputView& View)
	{
		return View.HasNative();
	}

	/// Returns the configured image resolution in pixels.
	UFUNCTION(BlueprintPure, Category = "OpenPLX Camera Color Output View")
	static FIntPoint GetResolution(const FOpenPLXCameraColorOutputView& View)
	{
		return View.GetResolution();
	}

	/// Returns the configured number of pixels.
	UFUNCTION(BlueprintPure, Category = "OpenPLX Camera Color Output View")
	static int32 GetNumPixels(const FOpenPLXCameraColorOutputView& View)
	{
		return View.GetNumPixels();
	}

	/// Returns the output channel element type.
	UFUNCTION(BlueprintPure, Category = "OpenPLX Camera Color Output View")
	static EAGX_CameraOutputChannelType GetChannelType(const FOpenPLXCameraColorOutputView& View)
	{
		return View.GetChannelType();
	}

	/// Returns the configured gamma correction value.
	UFUNCTION(BlueprintPure, Category = "OpenPLX Camera Color Output View")
	static double GetGamma(const FOpenPLXCameraColorOutputView& View)
	{
		return View.GetGamma();
	}

	/// Returns the matrix mapping linear RGB to the output channels.
	UFUNCTION(BlueprintPure, Category = "OpenPLX Camera Color Output View")
	static FAGX_ColorMappingMatrix GetColorMappingMatrix(const FOpenPLXCameraColorOutputView& View)
	{
		return View.GetColorMappingMatrix();
	}

	/// Returns the number of output channels per pixel.
	UFUNCTION(BlueprintPure, Category = "OpenPLX Camera Color Output View")
	static uint8 GetChannelCount(const FOpenPLXCameraColorOutputView& View)
	{
		return View.GetChannelCount();
	}

	/**
	 * Copy the output data as raw bytes.
	 *
	 * Only supports UInt8 and Float32 channel types. The output data is copied directly from the
	 * OpenPLX pixel buffer.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX Camera Color Output View")
	static bool GetDataBytes(
		const FOpenPLXCameraColorOutputView& View, TArray<uint8>& OutData)
	{
		return View.GetDataBytes(OutData);
	}

	/** Copy the output data as UInt8 channel values. Only supports UInt8 channel type. */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX Camera Color Output View")
	static bool GetDataU8(const FOpenPLXCameraColorOutputView& View, TArray<uint8>& OutData)
	{
		return View.GetDataU8(OutData);
	}

	/** Copy the output data as Float32 channel values. Only supports Float32 channel type. */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX Camera Color Output View")
	static bool GetDataF32(const FOpenPLXCameraColorOutputView& View, TArray<float>& OutData)
	{
		return View.GetDataF32(OutData);
	}

	/**
	 * Copy the underlying Camera Color output data into memory owned by this view.
	 * A newly received Camera Color output view references memory owned by the OpenPLX Control
	 * Interface and is only valid until another read reuses that buffer. Call this before storing the
	 * view for later use. This copies the complete Camera Color output buffer.
	 */
	UFUNCTION(BlueprintCallable, Category = "OpenPLX Camera Color Output View")
	static bool MakePersistant(UPARAM(Ref) FOpenPLXCameraColorOutputView& View)
	{
		return View.MakePersistant();
	}
};
