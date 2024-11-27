// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "ROS2/AGX_ROS2Messages.h"
#include "Sensors/AGX_CameraSensorBase.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <mutex>

#include "AGX_CameraSensor8BitComponent.generated.h"

class USceneCaptureComponent2D;
class UTextureRenderTarget2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewImagePixels8Bit, const TArray<FColor>&, Image);

/**
 * Camera Sensor Component, allowing to extract camera pixel information in runtime.
 * The captured image is encoded in 8-bit RGB wih linear color space.
 */
UCLASS(ClassGroup = "AGX_Sensor", Category = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_CameraSensor8BitComponent : public UAGX_CameraSensorBase
{
	GENERATED_BODY()

public:
	/**
	 * Tell the Camera to capture a new image as an array of 8-bit RGB pixels. This is an
	 * asynchronous operation and is faster than the blocking GetImagePixels which synchronizes with
	 * the render thread immediately. Bind to the NewImagePixels delegate to get a callback with the
	 * image data once it is ready.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Camera")
	void GetImagePixelsAsync();

	/**
	 * Delegate that is executed whenever a new Camera image as 8-bit RGB pixels is available.
	 * This delegate is called as the last step of a call to GetImagePixelsAsync.
	 * Users may bind to this delegate in order to get a callback.
	 *
	 * Note: all bound callbacks to this delegate are cleared on Level Transition meaning that
	 * objects surviving a Level Transition that also are bound to this delegates must bind to it
	 * again in the new Level.
	 */
	UPROPERTY(BlueprintAssignable, Category = "AGX Camera")
	FOnNewImagePixels8Bit NewImagePixels;

	/**
	 * Important: This may be a very slow operation. Use the Async version for better performance.
	 * Returns the current frame as seen by this Camera Sensor as an array of 8-bit RGB pixels.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Camera")
	TArray<FColor> GetImagePixels() const;

	/**
	 * Tell the Camera to capture a new image as a ROS2 sensor_msgs::Image message. This is an
	 * asynchronous operation and is faster than the blocking GetImageROS2 which synchronizes with
	 * the render thread immediately. Bind to the NewImageROS2 delegate to get a callback with the
	 * image message once it is ready.
	 * If Grayscale is set to true, only a single value (average intensity) for each pixel is
	 * set.
	 */
	UFUNCTION(
		BlueprintCallable, Category = "AGX Camera", meta = (DisplayName = "Get Image ROS2 Async"))
	void GetImageROS2Async(bool Grayscale);

	/**
	 * Delegate that is executed whenever a new image as a ROS2 sensor_msgs::Image message is
	 * available. This delegate is called as the last step of a call to GetImageROS2Async. Users may
	 * bind to this delegate in order to get a callback.
	 *
	 * Note: all bound callbacks to this delegate are cleared on Level Transition meaning that
	 * objects surviving a Level Transition that also are bound to this delegates must bind to it
	 * again in the new Level.
	 */
	UPROPERTY(BlueprintAssignable, Category = "AGX Camera")
	FOnNewImageROS2 NewImageROS2;

	/**
	 * Important: This may be a very slow operation. Use the Async version for better performance.
	 * Returns the current frame as seen by this Camera Sensor as a ROS2 sensor_msgs::Image message.
	 * If Grayscale is set to true, only a single value (average intensity) for each pixel is
	 * set.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Camera")
	FAGX_SensorMsgsImage GetImageROS2(bool Grayscale = false) const;

	//~ Begin UActorComponent Interface
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	//~ End UActorComponent Interface

	struct FAGX_ImageBuffer
	{
		TArray<FColor> Image[2]; // Buffer up to two images.
		int32 BufferHead {0}; // Points to one index in the Image buffer.
		std::mutex ImageMutex;
		bool EndPlayTriggered {false};
	};

private:
	TSharedPtr<FAGX_ImageBuffer> LastImage;

	virtual void Init() override;
	virtual bool CheckValid() const override;
	void OnAsyncImageRequest();
};
