// Copyright 2025, Algoryx Simulation AB.

#include "Sensors/AGX_CameraSensor8BitComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_Simulation.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_RenderUtilities.h"
#include "Utilities/AGX_ROS2Utilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "RenderingThread.h"
#include "TextureResource.h"

namespace AGX_CameraSensor8BitComponent_helpers
{
	struct FAGX_ImageAsyncParams
	{
		FAGX_ImageAsyncParams(
			const FIntPoint& InResolution, double InTimeStamp, bool InAsROS2Msg, bool InGrayscale)
			: Resolution(InResolution)
			, TimeStamp(InTimeStamp)
			, bAsROS2Msg(InAsROS2Msg)
			, bGrayscale(InGrayscale)
		{
		}

		FIntPoint Resolution;
		double TimeStamp {0.0};
		bool bAsROS2Msg {false};
		bool bGrayscale {false};
	};

	void GetImageAsync(
		UTextureRenderTarget2D* RenderTarget,
		TSharedPtr<UAGX_CameraSensor8BitComponent::FAGX_ImageBuffer> OutImg, int32 ImageIndex,
		FOnNewImagePixels8Bit& ImagePixelDelegate, FOnNewImageROS2& ImageROS2Delegate,
		const FAGX_ImageAsyncParams& Params)
	{
		if (RenderTarget == nullptr || OutImg == nullptr)
			return;

		const FIntRect Rectangle(0, 0, RenderTarget->SizeX, RenderTarget->SizeY);

		// Read the render target surface data back.
		struct FReadSurfaceContext
		{
			FRenderTarget* SrcRenderTarget;
			TArray<FColor>* OutData;
			FIntRect Rect;
			FReadSurfaceDataFlags Flags;
		};

		auto Rt = RenderTarget->GameThread_GetRenderTargetResource();
		FReadSurfaceContext Context = {
			Rt, &OutImg->Image[ImageIndex], Rectangle,
			FReadSurfaceDataFlags(RCM_MinMax, CubeFace_MAX)};

		Context.Flags.SetLinearToGamma(false);

		ENQUEUE_RENDER_COMMAND(FAGX_ReadRtCommand)
		(
			[Context, OutImg, ImageIndex, &ImagePixelDelegate, &ImageROS2Delegate,
			 Params](FRHICommandListImmediate& RHICmdList)
			{
				{
					std::scoped_lock<std::mutex> sl(OutImg->ImageMutex);
					RHICmdList.ReadSurfaceData(
						Context.SrcRenderTarget->GetRenderTargetTexture(), Context.Rect,
						*Context.OutData, Context.Flags);
				}

				// clang-format off
				FFunctionGraphTask::CreateAndDispatchWhenReady(
					[OutImg, ImageIndex, &ImagePixelDelegate, &ImageROS2Delegate, Params]()
					{
						std::lock_guard<std::mutex> lg(OutImg->ImageMutex);
						if (OutImg->EndPlayTriggered)
							return;
						if (Params.bAsROS2Msg)
							ImageROS2Delegate.Broadcast(FAGX_ROS2Utilities::Convert(
								OutImg->Image[ImageIndex], Params.TimeStamp, Params.Resolution, Params.bGrayscale));
						else
							ImagePixelDelegate.Broadcast(OutImg->Image[ImageIndex]);
					},
					TStatId {}, nullptr,
					ENamedThreads::GameThread);
				// clang-format on
			});
	}
}

void UAGX_CameraSensor8BitComponent::GetImagePixelsAsync()
{
	using namespace AGX_CameraSensor8BitComponent_helpers;

	if (!bIsValid || RenderTarget == nullptr)
		return;

	double TimeStamp = 0.0;
	if (UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(this))
	{
		TimeStamp = Sim->GetTimeStamp();
	}

	const FAGX_ImageAsyncParams Params(Resolution, TimeStamp, false, false);
	const int32 ImageIndex = LastImage->BufferHead;
	OnAsyncImageRequest();

	AGX_CameraSensor8BitComponent_helpers::GetImageAsync(
		RenderTarget, LastImage, ImageIndex, NewImagePixels, NewImageROS2, Params);
}

TArray<FColor> UAGX_CameraSensor8BitComponent::GetImagePixels() const
{
	if (!bIsValid)
		return TArray<FColor>();

	return UAGX_RenderUtilities::GetImagePixels8(RenderTarget);
}

void UAGX_CameraSensor8BitComponent::GetImageROS2Async(bool Grayscale)
{
	using namespace AGX_CameraSensor8BitComponent_helpers;

	if (!bIsValid || RenderTarget == nullptr)
		return;

	double TimeStamp = 0.0;
	if (UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(this))
	{
		TimeStamp = Sim->GetTimeStamp();
	}

	const FAGX_ImageAsyncParams Params(Resolution, TimeStamp, true, Grayscale);
	const int32 ImageIndex = LastImage->BufferHead;
	OnAsyncImageRequest();
	GetImageAsync(RenderTarget, LastImage, ImageIndex, NewImagePixels, NewImageROS2, Params);
}

FAGX_SensorMsgsImage UAGX_CameraSensor8BitComponent::GetImageROS2(bool Grayscale) const
{
	if (!bIsValid)
		return FAGX_SensorMsgsImage();

	double TimeStamp = 0.0;
	if (UAGX_Simulation* Sim = UAGX_Simulation::GetFrom(this))
		TimeStamp = Sim->GetTimeStamp();

	return UAGX_RenderUtilities::GetImageROS2(RenderTarget, TimeStamp, Grayscale);
}

void UAGX_CameraSensor8BitComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);

	NewImagePixels.Clear();
	NewImageROS2.Clear();

	if (LastImage != nullptr)
		LastImage->EndPlayTriggered = true;
}

void UAGX_CameraSensor8BitComponent::Init()
{
	Super::Init();

	if (LastImage == nullptr)
	{
		LastImage = MakeShared<FAGX_ImageBuffer>();
	}
}

bool UAGX_CameraSensor8BitComponent::CheckValid() const
{
	if (!Super::CheckValid())
		return false;

	if (RenderTarget->GetFormat() != EPixelFormat::PF_B8G8R8A8)
	{
		const FString Msg = FString::Printf(
			TEXT("Camera Sensor '%s' in Actor '%s' has a RenderTarget with an unsupported pixel "
				 "format. The pixel format should be RGBA8. Use the 'Generate Runtime Assets' "
				 "button in the Details Panel to generate a valid RenderTarget."),
			*GetName(), *GetLabelSafe(GetOwner()));
		FAGX_NotificationUtilities::ShowNotification(Msg, SNotificationItem::CS_Fail);
		return false;
	}

	return true;
}

void UAGX_CameraSensor8BitComponent::OnAsyncImageRequest()
{
	AGX_CHECK(LastImage->BufferHead < 2);
	LastImage->BufferHead = (LastImage->BufferHead + 1) % 2;
}
