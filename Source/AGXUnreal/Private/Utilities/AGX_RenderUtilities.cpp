// Copyright 2024, Algoryx Simulation AB.

#include "Utilities/AGX_RenderUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Contacts/ShapeContactBarrier.h"
#include "ROS2/AGX_ROS2Messages.h"
#include "Utilities/AGX_ROS2Utilities.h"

// Unreal Engine includes.
#include "DrawDebugHelpers.h"
#include "Misc/EngineVersionComparison.h"
#if !UE_VERSION_OLDER_THAN(5, 2, 0)
// Possible include loop in Unreal Engine.
// - Engine/TextureRenderTarget2D.h
// - RenderUtils.h
// - RHIShaderPlatform.h
//     Defines FStaticShaderPlatform, but includes RHIDefinitions.h first.
// - RHIDefinitions.h
// - DataDrivenShaderPlatformInfo.h
//   Needs FStaticShaderPlatform so includes RHIShaderPlatform.h. But that file is already being
//   included so ignored. So FStaticShaderPlatform will be defined soon, but it isn't yet. So
//   the compile fails.
//
// We work around this by including DataDrivenShaderPlatformInfo.h ourselves before all of the
// above. Now DataDrivenShaderPlatformInfo.h can include RHIShaderPlatform.h succesfully and
// FStaticShaderPlatform is defined when DataDrivenShaderPlatformInfo.h needs it. When we include
// DynamicMeshBuild.h shortly most of the include files are skipped because they have already been
// included as part of DataDrivenShaderPlatformInfo.h here.
#include "DataDrivenShaderPlatformInfo.h"
#endif
#include "Engine/StaticMesh.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/Material.h"
#include "Misc/EngineVersionComparison.h"
#include "RenderingThread.h"
#include "RHI.h"
#include "RHICommandList.h"
#include "TextureResource.h"

bool FAGX_RenderUtilities::UpdateRenderTextureRegions(
	UTextureRenderTarget2D& RenderTarget, uint32 NumRegions, FUpdateTextureRegion2D* Regions,
	uint32 SourcePitch, uint32 SourceBitsPerPixel, uint8* SourceData, bool bFreeData)
{
#if UE_VERSION_OLDER_THAN(5, 0, 0)
	FTextureRenderTarget2DResource* Resource =
		(FTextureRenderTarget2DResource*) (RenderTarget.Resource);
#else
	FTextureRenderTarget2DResource* Resource =
		(FTextureRenderTarget2DResource*) (RenderTarget.GetResource());
#endif
	if (Resource == nullptr)
	{
		UE_LOG(LogAGX, Error, TEXT("TextureRenderTarget doesn't have a resource."));
		return false;
	}

	auto WriteTexture = [Resource, NumRegions, Regions, SourcePitch, SourceBitsPerPixel, SourceData,
						 bFreeData](FRHICommandListImmediate& RHICmdList)
	{
		FRHITexture* Texture = Resource->GetTextureRHI();
		for (uint32 RegionIndex = 0; RegionIndex < NumRegions; ++RegionIndex)
		{
			FUpdateTextureRegion2D& Region = Regions[RegionIndex];
			uint8* Bits = SourceData + Region.SrcY * SourcePitch + Region.SrcX * SourceBitsPerPixel;
			RHIUpdateTexture2D(Texture, /*MipIndex*/ 0, Region, SourcePitch, Bits);
		}
		if (bFreeData)
		{
			FMemory::Free(Regions);
			FMemory::Free(SourceData);
		}
	};

	ENQUEUE_RENDER_COMMAND(UpdateRenderTextureRegionsData)(std::move(WriteTexture));

	return true;
}

void FAGX_RenderUtilities::DrawContactPoints(
	const TArray<FShapeContactBarrier>& ShapeContacts, float LifeTime, UWorld* World)
{
	for (const FShapeContactBarrier& ShapeContact : ShapeContacts)
	{
		for (const FContactPointBarrier& ContactPoint : ShapeContact.GetContactPoints())
		{
			const FVector PointLocation = ContactPoint.GetLocation();
			DrawDebugSphere(World, PointLocation, 1.5f, 10, FColor::Orange, false, LifeTime, 99);

			// The Normal is drawn as a 3.5 cm line.
			const FVector NormalEnd = PointLocation + ContactPoint.GetNormal() * 3.5;
			DrawDebugLine(
				World, PointLocation, NormalEnd, FColor::Orange, false, LifeTime, 99, 0.35f);
		}
	}
}

TArray<FColor> UAGX_RenderUtilities::GetImagePixels8(UTextureRenderTarget2D* RenderTarget)
{
	if (RenderTarget == nullptr || RenderTarget->GetFormat() != EPixelFormat::PF_B8G8R8A8)
		return TArray<FColor>();

	FTextureRenderTargetResource* RtResource = RenderTarget->GameThread_GetRenderTargetResource();
	TArray<FColor> PixelData;
	FReadSurfaceDataFlags Flags(RCM_MinMax, CubeFace_MAX);
	Flags.SetLinearToGamma(false);


	// ReadPixels will synchronize (wait) for the render thread to finish, making it slow.
	RtResource->ReadPixels(PixelData, Flags);
	return PixelData;
}

TArray<FFloat16Color> UAGX_RenderUtilities::GetImagePixels16(UTextureRenderTarget2D* RenderTarget)
{
	if (RenderTarget == nullptr || RenderTarget->GetFormat() != EPixelFormat::PF_FloatRGBA)
		return TArray<FFloat16Color>();

	FTextureRenderTargetResource* RtResource = RenderTarget->GameThread_GetRenderTargetResource();
	TArray<FFloat16Color> PixelData;

	// TODO: once UE5.4 is the oldest version we support, simply use the new ReadFloat16Pixels with
	// the linear color space parameter.
	const FIntRect Rectangle(0, 0, RenderTarget->SizeX, RenderTarget->SizeY);

	// Read the render target surface data back.
	struct FReadSurfaceContext
	{
		FRenderTarget* SrcRenderTarget;
		TArray<FFloat16Color>* OutData;
		FIntRect Rect;
		FReadSurfaceDataFlags Flags;
	};

	auto Rt = RenderTarget->GameThread_GetRenderTargetResource();
	FReadSurfaceContext Context = {
		Rt, &PixelData, Rectangle, FReadSurfaceDataFlags(RCM_MinMax, CubeFace_MAX)};

	Context.Flags.SetLinearToGamma(false);

	ENQUEUE_RENDER_COMMAND(FAGX_ReadRtCommand16Util)
	(
		[Context](FRHICommandListImmediate& RHICmdList)
		{
			RHICmdList.ReadSurfaceFloatData(
				Context.SrcRenderTarget->GetRenderTargetTexture(), Context.Rect, *Context.OutData,
				Context.Flags);
		}
	);
	FlushRenderingCommands();

	return PixelData;
}

namespace AGX_RenderUtilities_helpers
{
	template <typename PixelType>
	FAGX_SensorMsgsImage GetImageROS2(
		const TArray<PixelType>& Pixels, const FIntPoint& Resolution, double TimeStamp,
		bool Grayscale)
	{
		if (Pixels.Num() == 0)
			return FAGX_SensorMsgsImage();

		return FAGX_ROS2Utilities::Convert(Pixels, TimeStamp, Resolution, Grayscale);
	}
}

FAGX_SensorMsgsImage UAGX_RenderUtilities::GetImageROS2(
	UTextureRenderTarget2D* RenderTarget, double TimeStamp, bool Grayscale)
{
	const FIntPoint Resolution(RenderTarget->SizeX, RenderTarget->SizeY);

	if (RenderTarget->GetFormat() == EPixelFormat::PF_B8G8R8A8)
	{
		return AGX_RenderUtilities_helpers::GetImageROS2(
			GetImagePixels8(RenderTarget), Resolution, TimeStamp, Grayscale);
	}
	else if (RenderTarget->GetFormat() == EPixelFormat::PF_FloatRGBA)
	{
		return AGX_RenderUtilities_helpers::GetImageROS2(
			GetImagePixels16(RenderTarget), Resolution, TimeStamp, Grayscale);
	}

	return FAGX_SensorMsgsImage();
}
