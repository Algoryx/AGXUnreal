// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_CameraEnums.h"
#include "Sensors/AGX_ColorMappingMatrix.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

#include "OpenPLXCameraColorOutputView.generated.h"

struct FOpenPLXCameraColorOutputViewRef;

/**
 * View into Camera Color output data received through OpenPLX.
 *
 * By default this struct references memory owned by the OpenPLX Control Interface. No Camera Color
 * output data is copied until data is requested. A newly received view is only valid until another
 * read operation reuses the underlying Control Interface buffer. Call MakePersistant before storing
 * the view for later use.
 */
USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FOpenPLXCameraColorOutputView
{
	GENERATED_BODY()

	FOpenPLXCameraColorOutputView();
	FOpenPLXCameraColorOutputView(std::shared_ptr<FOpenPLXCameraColorOutputViewRef> Native);

	bool HasNative() const;

	/// Returns the configured image resolution in pixels.
	FIntPoint GetResolution() const;

	/// Returns the number of pixels in the received output buffer.
	int32 GetNumPixels() const;

	/// Returns the output channel element type.
	EAGX_CameraOutputChannelType GetChannelType() const;

	/// Returns the configured gamma correction value.
	double GetGamma() const;

	/// Returns the matrix mapping linear RGB to the output channels.
	FAGX_ColorMappingMatrix GetColorMappingMatrix() const;

	/// Returns the number of output channels per pixel.
	uint8 GetChannelCount() const;

	/**
	 * Copy the output data as raw bytes.
	 *
	 * Only supports UInt8 and Float32 channel types. The output data is copied directly from the
	 * OpenPLX pixel buffer.
	 */
	bool GetDataBytes(TArray<uint8>& OutData) const;

	/** Copy the raw output data directly into a caller-provided buffer. */
	bool CopyDataBytesTo(void* OutData, uint64 OutDataSize) const;

	/**
	 * Copy the output data as UInt8 channel values.
	 *
	 * Only supports UInt8 channel type. The output data is copied directly from the OpenPLX pixel
	 * buffer.
	 */
	bool GetDataU8(TArray<uint8>& OutData) const;

	/**
	 * Copy the output data as Float32 channel values.
	 *
	 * Only supports Float32 channel type. The output data is copied directly from the OpenPLX pixel
	 * buffer.
	 */
	bool GetDataF32(TArray<float>& OutData) const;

	/**
	 * Copy the underlying Camera Color output data into memory owned by this view.
	 * A newly received Camera Color output view references memory owned by the OpenPLX Control
	 * Interface and is only valid until another read reuses that buffer. Call this before storing
	 * the view for later use. This copies the complete Camera Color output buffer.
	 */
	bool MakePersistant();

	FOpenPLXCameraColorOutputViewRef* GetNative();
	const FOpenPLXCameraColorOutputViewRef* GetNative() const;

private:
	std::shared_ptr<FOpenPLXCameraColorOutputViewRef> NativeRef;
};
