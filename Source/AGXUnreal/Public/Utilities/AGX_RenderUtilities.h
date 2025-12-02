// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_RenderUtilities.generated.h"

class FShapeContactBarrier;
class UTextureRenderTarget2D;
class UMaterial;
class UStaticMesh;

struct FAGX_SensorMsgsImage;
struct FUpdateTextureRegion2D;

class AGXUNREAL_API FAGX_RenderUtilities
{
public:
	static bool UpdateRenderTextureRegions(
		UTextureRenderTarget2D& Texture, uint32 NumRegions, FUpdateTextureRegion2D* Regions,
		uint32 SourcePitch, uint32 SourceBitsPerPixel, uint8* SourceData, bool bFreeData);

	/**
	 * Renders the given ShapeContacts to the screen.
	 * The rendering is not avaiable in built applications built with Shipping configuration.
	 */
	static void DrawContactPoints(
		const TArray<FShapeContactBarrier>& ShapeContacts, float LifeTime, UWorld* World);
};

UCLASS(ClassGroup = "AGX Render Utilities")
class AGXUNREAL_API UAGX_RenderUtilities : public UBlueprintFunctionLibrary
{
public:
	GENERATED_BODY()

	/**
	 * Important: This may be a very slow operation.
	 * The given RenderTarget must use the format RGBA8.
	 * Returns the image pixels given a Render Target as an array of 8-bit RGB pixels.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Utilities")
	static TArray<FColor> GetImagePixels8(UTextureRenderTarget2D* RenderTarget);

	/**
	 * Important: This may be a very slow operation.
	 * The given RenderTarget must use the format RGBA16f.
	 * Returns the image pixels given a Render Target as an array of 16-bit RGB pixels.
	 * FFloat16Color is not supported by Blueprints.
	 */
	static TArray<FFloat16Color> GetImagePixels16(UTextureRenderTarget2D* RenderTarget);

	/**
	 * Important: This may be a very slow operation.
	 * The given RenderTarget must use the format RGBA8 or RGBA16f.
	 * Returns the image pixels given a Render Target as a ROS2 sensor_msgs::Image message.
	 * If the given Render Target uses RGBA8 the pixel data is stored as 8-bit values, and if the
	 * Render Target uses RGBA16 the pixel data is stored as 16-bit values, little endian.
	 * If Grayscale is set to true, only a single value (average intensity) for each pixel is set.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Utilities")
	static FAGX_SensorMsgsImage GetImageROS2(
		UTextureRenderTarget2D* RenderTarget, double TimeStamp, bool Grayscale = false);
};
