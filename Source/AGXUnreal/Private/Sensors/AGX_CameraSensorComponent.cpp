// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_CameraSensorComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "Import/AGX_ImportContext.h"
#include "Sensors/AGX_CameraBackend.h"
#include "Sensors/AGX_CameraCMOSSensor.h"
#include "Sensors/AGX_CameraLensBase.h"
#include "Sensors/AGX_CameraLensSingleElement.h"
#include "Sensors/AGX_CameraOutputBase.h"
#include "Sensors/AGX_CameraPhotodetectorBase.h"
#include "Sensors/AGX_SensorEnvironmentSubsystem.h"
#include "Sensors/CameraBackendBarrier.h"
#include "Sensors/CameraBarrier.h"
#include "Sensors/CameraCMOSSensorBarrier.h"
#include "Sensors/CameraLensBarrier.h"
#include "Sensors/CameraLensSingleElementBarrier.h"
#include "Sensors/CameraOutputBarrier.h"
#include "Sensors/CameraOutputColorBarrier.h"
#include "Sensors/CameraPhotodetectorBarrier.h"
#include "Sensors/LensDistortionBrownConradyBarrier.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "CanvasItem.h"
#include "CollisionQueryParams.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/Canvas.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "PixelFormat.h"
#include "RHI.h"
#include "RenderingThread.h"

UAGX_CameraSensorComponent::UAGX_CameraSensorComponent()
{
	NativeBarrier.Reset(new FCameraBarrier());
	PrimaryComponentTick.bCanEverTick = true;

	static const TCHAR* CameraPass1AssetPath = TEXT(
		"Material'/AGXUnreal/Sensor/Camera/Materials/MI_AGX_Camera_Pass1.MI_AGX_Camera_Pass1'");
	MaterialPasses.Add(
		FAGX_ObjectUtilities::GetAssetFromPath<UMaterialInterface>(CameraPass1AssetPath));
}

namespace AGX_CameraSensorComponent_helpers
{
	void SetLocalScope(UAGX_CameraSensorComponent& Component)
	{
		AActor* const Owner = FAGX_ObjectUtilities::GetRootParentActor(Component);
		Component.CaptureSourceOverride.LocalScope = Owner;
	}

	float CalculateHorizontalFOVDegrees(double SensorWidth, double FocalLength)
	{
		if (SensorWidth <= 0.0 || FocalLength <= 0.0)
			return 0.0f;

		return static_cast<float>(
			FMath::RadiansToDegrees(2.0 * FMath::Atan(SensorWidth / (2.0 * FocalLength))));
	}

	const UAGX_CameraCMOSSensor& GetCMOSSensorOrDefault(const UAGX_CameraSensorComponent& Component)
	{
		if (const UAGX_CameraCMOSSensor* Sensor =
				Cast<UAGX_CameraCMOSSensor>(Component.PhotoDetector))
		{
			return *Sensor;
		}

		return *GetDefault<UAGX_CameraCMOSSensor>();
	}

	const UAGX_CameraLensSingleElement& GetCameraLensSingleElementOrDefault(
		const UAGX_CameraSensorComponent& Component)
	{
		if (const UAGX_CameraLensSingleElement* Lens =
				Cast<UAGX_CameraLensSingleElement>(Component.CameraLens))
		{
			return *Lens;
		}

		return *GetDefault<UAGX_CameraLensSingleElement>();
	}

	double CalculateAutofocusDistance(
		const USceneCaptureComponent2D& SceneCapture, double MinimumFocusDistance)
	{
		// For SceneCaptureComponent2D, FocalDistance of zero disables it.
		const double ClampedMinimumFocusDistance = FMath::Max(0.1, MinimumFocusDistance);

		UWorld* World = SceneCapture.GetWorld();
		if (World == nullptr)
			return ClampedMinimumFocusDistance;

		const FVector Start = SceneCapture.GetComponentLocation();
		const FVector Direction = SceneCapture.GetForwardVector().GetSafeNormal();
		if (Direction.IsNearlyZero())
			return ClampedMinimumFocusDistance;

		FCollisionQueryParams QueryParams(
			SCENE_QUERY_STAT(AGXCameraAutofocus), /*bTraceComplex*/ true);
		constexpr double AutofocusTraceDistance = 1000000.0; // A bit arbitrarily chosen.
		const FVector End = Start + Direction * AutofocusTraceDistance;
		FHitResult Hit;
		if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, QueryParams))
			return FMath::Max(ClampedMinimumFocusDistance, static_cast<double>(Hit.Distance));

		return FMath::Max(ClampedMinimumFocusDistance, AutofocusTraceDistance);
	}

	void UpdateLensFocalDistance(
		USceneCaptureComponent2D& SceneCapture, bool bUseAutofocus, double MinimumFocusDistance,
		double FocusDistance)
	{
		FPostProcessSettings& PostProcessSettings = SceneCapture.PostProcessSettings;
		if (bUseAutofocus)
		{
			PostProcessSettings.DepthOfFieldFocalDistance = static_cast<float>(
			CalculateAutofocusDistance(SceneCapture, MinimumFocusDistance));
		}
		else
		{
			PostProcessSettings.DepthOfFieldFocalDistance = static_cast<float>(FocusDistance);
		}
	}

	void UpdateLensFocalDistance(
		USceneCaptureComponent2D& SceneCapture, const UAGX_CameraLensSingleElement& Lens)
	{
		UpdateLensFocalDistance(
			SceneCapture, Lens.GetUseAutofocus(), Lens.GetMinimumFocusDistance(),
			Lens.GetFocusDistance());
	}

	bool IsSupportedReadbackFormat(EPixelFormat PixelFormat)
	{
		switch (PixelFormat)
		{
			case PF_G8: // For U8, 1 channel.
			case PF_R8G8: // For U8, 2 channels.
			case PF_B8G8R8A8: // For U8, 3 and 4 channels.
			case PF_R8G8B8A8: // For U8, 3 and 4 channels.
			case PF_R32_FLOAT: // For F32, 1 channel.
			case PF_G32R32F: // For F32, 2 channels.
			case PF_A32B32G32R32F: // For F32, 3 and 4 channels.
				return true;
			default:
				return false;
		}
	}

	TOptional<ETextureRenderTargetFormat> GetRenderTargetFormat(
		EAGX_CameraOutputChannelType ChannelType, uint8 ChannelCount)
	{
		if (ChannelCount < 1 || ChannelCount > 4)
			return {};

		switch (ChannelType)
		{
			case EAGX_CameraOutputChannelType::U8:
				switch (ChannelCount)
				{
					case 1:
						return ETextureRenderTargetFormat::RTF_R8;
					case 2:
						return ETextureRenderTargetFormat::RTF_RG8;
					default:
						return ETextureRenderTargetFormat::RTF_RGBA8;
				}

			case EAGX_CameraOutputChannelType::F32:
				switch (ChannelCount)
				{
					case 1:
						return ETextureRenderTargetFormat::RTF_R32f;
					case 2:
						return ETextureRenderTargetFormat::RTF_RG32f;
					default:
						return ETextureRenderTargetFormat::RTF_RGBA32f;
				}

			default:
				return {};
		}
	}

	TOptional<int32> GetChannelSize(EAGX_CameraOutputChannelType ChannelType)
	{
		switch (ChannelType)
		{
			case EAGX_CameraOutputChannelType::U8:
				return sizeof(uint8);
			case EAGX_CameraOutputChannelType::F32:
				return sizeof(float);
			default:
				return {};
		}
	}

	FLensDistortionBrownConradyBarrier GetLensDistortionBrownConradyBarrier(
		const UAGX_CameraSensorComponent& Component)
	{
		if (Component.CameraLens == nullptr)
			return FLensDistortionBrownConradyBarrier();

		const UAGX_CameraLensSingleElement* SingleElementLens =
			Cast<UAGX_CameraLensSingleElement>(Component.CameraLens);
		if (SingleElementLens == nullptr)
			return FLensDistortionBrownConradyBarrier();

		const FCameraLensSingleElementBarrier* LensBarrier =
			SingleElementLens->GetNativeAsSingleElement();
		if (LensBarrier == nullptr)
			return FLensDistortionBrownConradyBarrier();

		return LensBarrier->GetLensDistortionBrownConrady();
	}

	FVector2D GetUndistortedPoint(
		const FVector2D& DistortedPoint, double K1, double K2, double K3, double P1, double P2)
	{
		FVector2D Point = DistortedPoint;
		constexpr int32 NumIterations = 10;
		constexpr double ErrorThreshold = 1.0e-4;
		for (int32 Iteration = 0; Iteration < NumIterations; ++Iteration)
		{
			const double X = Point.X;
			const double Y = Point.Y;
			const double R2 = X * X + Y * Y;
			const double R4 = R2 * R2;
			const double R6 = R4 * R2;
			const double Radial = 1.0 + K1 * R2 + K2 * R4 + K3 * R6;
			const double TangentialX = 2.0 * P1 * X * Y + P2 * (R2 + 2.0 * X * X);
			const double TangentialY = 2.0 * P2 * X * Y + P1 * (R2 + 2.0 * Y * Y);

			const FVector2D NextPoint(
				(DistortedPoint.X - TangentialX) / Radial,
				(DistortedPoint.Y - TangentialY) / Radial);
			const double DeltaX = FMath::Abs(NextPoint.X - Point.X);
			const double DeltaY = FMath::Abs(NextPoint.Y - Point.Y);
			Point = NextPoint;

			if (DeltaX < ErrorThreshold && DeltaY < ErrorThreshold)
				break;
		}

		return Point;
	}

	void CalculateLensDistortionCompensation(
		float InFOV, const FIntPoint& InResolution,
		const FLensDistortionBrownConradyBarrier* LensDistortion, float& OutFOV,
		FIntPoint& OutResolution)
	{
		OutFOV = InFOV;
		OutResolution = InResolution;

		if (InFOV <= 0.0f || InResolution.X <= 0 || InResolution.Y <= 0 ||
			LensDistortion == nullptr || !LensDistortion->HasNative())
		{
			return;
		}

		const double K1 = LensDistortion->GetK1();
		const double K2 = LensDistortion->GetK2();
		const double K3 = LensDistortion->GetK3();
		const double P1 = LensDistortion->GetP1();
		const double P2 = LensDistortion->GetP2();
		if (K1 == 0.0 && K2 == 0.0 && K3 == 0.0 && P1 == 0.0 && P2 == 0.0)
			return;

		const double AspectRatio =
			static_cast<double>(InResolution.X) / static_cast<double>(InResolution.Y);
		const double HalfWidth =
			FMath::Tan(FMath::DegreesToRadians(static_cast<double>(InFOV) * 0.5));
		const double HalfHeight = HalfWidth / AspectRatio;
		if (HalfWidth <= 0.0 || HalfHeight <= 0.0)
			return;

		double RequiredScale = 1.0;
		const auto VisitPoint = [&RequiredScale, HalfWidth, HalfHeight, K1, K2, K3, P1,
								 P2](const FVector2D& Point)
		{
			const FVector2D UndistortedPoint =
				GetUndistortedPoint(Point, K1, K2, K3, P1, P2);
			const double ScaleX = FMath::Abs(UndistortedPoint.X) / HalfWidth;
			const double ScaleY = FMath::Abs(UndistortedPoint.Y) / HalfHeight;
			if (FMath::IsFinite(ScaleX))
				RequiredScale = FMath::Max(RequiredScale, ScaleX);
			if (FMath::IsFinite(ScaleY))
				RequiredScale = FMath::Max(RequiredScale, ScaleY);
		};

		constexpr int32 NumBoundarySegments = 16;
		for (int32 Index = 0; Index <= NumBoundarySegments; ++Index)
		{
			const double T =
				-1.0 + 2.0 * static_cast<double>(Index) / static_cast<double>(NumBoundarySegments);
			VisitPoint(FVector2D(HalfWidth, T * HalfHeight));
			VisitPoint(FVector2D(-HalfWidth, T * HalfHeight));
			VisitPoint(FVector2D(T * HalfWidth, HalfHeight));
			VisitPoint(FVector2D(T * HalfWidth, -HalfHeight));
		}

		if (RequiredScale <= 1.0)
			return;

		OutFOV = static_cast<float>(
			FMath::RadiansToDegrees(2.0 * FMath::Atan(HalfWidth * RequiredScale)));
		OutResolution = FIntPoint(
			FMath::Max(1, FMath::CeilToInt(static_cast<double>(InResolution.X) * RequiredScale)),
			FMath::Max(1, FMath::CeilToInt(static_cast<double>(InResolution.Y) * RequiredScale)));
	}
}

void UAGX_CameraSensorComponent::AddMaterialPass(UMaterialInterface* Material)
{
	MaterialPasses.Add(Material);
}

bool UAGX_CameraSensorComponent::SetMaterialPass(int32 Index, UMaterialInterface* Material)
{
	if (!MaterialPasses.IsValidIndex(Index))
		return false;

	MaterialPasses[Index] = Material;
	return true;
}

bool UAGX_CameraSensorComponent::RemoveMaterialPass(UMaterialInterface* Material)
{
	const int32 NumRemoved =
		MaterialPasses.RemoveAll([Material](const TObjectPtr<UMaterialInterface>& ExistingMaterial)
								 { return ExistingMaterial.Get() == Material; });
	if (NumRemoved == 0)
		return false;

	return true;
}

bool UAGX_CameraSensorComponent::RemoveMaterialPassAt(int32 Index)
{
	if (!MaterialPasses.IsValidIndex(Index))
		return false;

	MaterialPasses.RemoveAt(Index);
	return true;
}

void UAGX_CameraSensorComponent::ClearMaterialPasses()
{
	MaterialPasses.Empty();
}

void UAGX_CameraSensorComponent::UpdateNativeTransform()
{
	if (HasNative())
		GetNativeAsCamera()->SetTransform(GetComponentTransform());
}

bool UAGX_CameraSensorComponent::AddOutput(FAGX_CameraOutputBase& InOutput)
{
	if (bOpenPLXImported)
	{
		FAGX_NotificationUtilities::ShowNotification(
			FString::Printf(
				TEXT("Outputs cannot be manually added to Camera Sensor '%s' in '%s' because it "
					 "was imported from an OpenPLX file which define its outputs. OpenPLX outputs "
					 "are added automatically at BeginPlay for this Camera Sensor."),
				*GetName(), *GetLabelSafe(GetOwner())),
			SNotificationItem::CS_Fail);
		return false;
	}

	FCameraBarrier* Native = GetNativeAsCamera();
	if (Native == nullptr)
		return false;

	FCameraOutputBarrier* OutputNative = InOutput.GetOrCreateNative();
	if (OutputNative == nullptr)
		return false;

	Native->AddOutput(*OutputNative);

	// If SetCameraColorOutput would have been triggered in the CameraBackend on AddOutput,
	// we this would be handled for us in OnBackendSetCameraColorOutput. But that gets called
	// later, and the user may need the Output RenderTarget on BeginPlay, therfore we call this
	// here immediately to ensure we are all set up.
	if (FCameraOutputColorBarrier::IsColorOutput(*OutputNative))
	{
		FCameraOutputColorBarrier& ColorOutputNative =
			static_cast<FCameraOutputColorBarrier&>(*OutputNative);
		UpdateOutputCaptureSettings(ColorOutputNative);
	}

	return true;
}

void UAGX_CameraSensorComponent::SetCaptureSourceOverride(
	USceneCaptureComponent2D* InCaptureSourceOverride)
{
	AGX_CameraSensorComponent_helpers::SetLocalScope(*this);
	CaptureSourceOverride.SetComponent(InCaptureSourceOverride);
}

bool UAGX_CameraSensorComponent::HasCaptureSourceOverride() const
{
	return CaptureSourceOverride.GetSceneCaptureComponent2D() != nullptr;
}

USceneCaptureComponent2D* UAGX_CameraSensorComponent::GetCaptureSource() const
{
	if (USceneCaptureComponent2D* CaptureSource =
			CaptureSourceOverride.GetSceneCaptureComponent2D())
	{
		return CaptureSource;
	}

	return OwnedCaptureComponent2D;
}

bool UAGX_CameraSensorComponent::IsCameraSensorValid() const
{
	USceneCaptureComponent2D* CaptureSource = GetCaptureSource();
	if (CaptureSource == nullptr || !HasNative())
		return false;

	if (HasCaptureSourceOverride() && CaptureSource->TextureTarget == nullptr)
		return false;

	return true;
}

UTextureRenderTarget2D* UAGX_CameraSensorComponent::RenderMaterialPasses(
	FCameraOutputRenderContext& OutputRenderContext,
	const FCameraOutputColorBarrier& OutputColorBarrier)
{
	USceneCaptureComponent2D* CaptureSource = GetCaptureSource();
	if (CaptureSource == nullptr)
		return nullptr;

	AGX_CHECK(OutputRenderContext.SceneRenderTarget != nullptr);
	if (OutputRenderContext.SceneRenderTarget == nullptr)
		return nullptr;

	if (!CaptureSource->bCaptureEveryFrame)
		CaptureSource->CaptureScene();

	UTexture* InputTexture = OutputRenderContext.SceneRenderTarget.Get();
	UTextureRenderTarget2D* FinalRenderTarget = OutputRenderContext.SceneRenderTarget.Get();
	AGX_CHECK(
		OutputRenderContext.MaterialInstances.Num() == OutputRenderContext.RenderTargets.Num());
	for (int32 Index = 0; Index < OutputRenderContext.MaterialInstances.Num(); ++Index)
	{
		UMaterialInstanceDynamic* MaterialInstance = OutputRenderContext.MaterialInstances[Index];
		if (MaterialInstance == nullptr)
			continue;

		AGX_CHECK(OutputRenderContext.RenderTargets.IsValidIndex(Index));
		if (!OutputRenderContext.RenderTargets.IsValidIndex(Index) ||
			OutputRenderContext.RenderTargets[Index] == nullptr)
			continue;

		UTextureRenderTarget2D* OutputRenderTarget = OutputRenderContext.RenderTargets[Index].Get();
		MaterialInstance->SetTextureParameterValue(TEXT("InputTexture"), InputTexture);

		UCanvas* Canvas = nullptr;
		FVector2D CanvasSize(0.0f, 0.0f);
		FDrawToRenderTargetContext DrawContext;
		UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(
			this, OutputRenderTarget, Canvas, CanvasSize, DrawContext);

		if (Canvas != nullptr)
		{
			FCanvasTileItem Item(
				FVector2D::ZeroVector, MaterialInstance->GetRenderProxy(),
				FVector2D(OutputRenderTarget->SizeX, OutputRenderTarget->SizeY),
				FVector2D(0.0f, 0.0f), FVector2D(1.0f, 1.0f));
			Item.BlendMode = SE_BLEND_Opaque;
			Canvas->DrawItem(Item);
		}

		UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, DrawContext);

		FinalRenderTarget = OutputRenderTarget;
		InputTexture = FinalRenderTarget;
	}

	return FinalRenderTarget;
}

UTextureRenderTarget2D* UAGX_CameraSensorComponent::GetOutputRenderTarget(
	const FAGX_CameraOutputColor& Output) const
{
	const FCameraOutputBarrier* Native = Output.GetNative();
	if (Native == nullptr)
		return nullptr;

	const FCameraOutputRenderContext* Context =
		OutputRenderContexts.Find(Native->GetNativeAddress());
	if (Context == nullptr)
		return nullptr;

	for (int32 Index = Context->RenderTargets.Num() - 1; Index >= 0; --Index)
	{
		if (Context->RenderTargets[Index] != nullptr)
			return Context->RenderTargets[Index].Get();
	}

	return Context->SceneRenderTarget.Get();
}

FCameraOutputRenderContext* UAGX_CameraSensorComponent::GetOrCreateOutputRenderContext(
	const FCameraOutputColorBarrier& OutputColorBarrier)
{
	if (!OutputColorBarrier.HasNative())
		return nullptr;

	return &OutputRenderContexts.FindOrAdd(OutputColorBarrier.GetNativeAddress());
}

FCameraOutputRenderContext* UAGX_CameraSensorComponent::UpdateOutputCaptureSettings(
	const FCameraOutputColorBarrier& OutputColorBarrier, bool bLogWarnings)
{
	FCameraOutputRenderContext* OutputRenderContext =
		GetOrCreateOutputRenderContext(OutputColorBarrier);
	if (OutputRenderContext == nullptr)
		return nullptr;

	return UpdateOutputRenderContextNoParams(
			*OutputRenderContext, OutputColorBarrier, bLogWarnings)
			? OutputRenderContext
			: nullptr;
}

void UAGX_CameraSensorComponent::UpdateAllOutputCaptureSettings()
{
	FCameraBarrier* CameraBarrier = GetNativeAsCamera();
	if (CameraBarrier == nullptr)
		return;

	TArray<FCameraOutputBarrier> OutputBarriers = CameraBarrier->GetOutputs();
	for (FCameraOutputBarrier& OutputBarrier : OutputBarriers)
	{
		if (!FCameraOutputColorBarrier::IsColorOutput(OutputBarrier))
			continue;

		const FCameraOutputColorBarrier OutputColorBarrier =
			FCameraOutputColorBarrier::CreateFrom(OutputBarrier);
		UpdateOutputCaptureSettings(OutputColorBarrier);
	}
}

void UAGX_CameraSensorComponent::UpdateMaterialParametersFrom(
	const FCameraOutputColorBarrier& OutputColorBarrier,
	TArray<TObjectPtr<UMaterialInstanceDynamic>>& OutMaterials)
{
	for (auto& Material : OutMaterials)
	{
		if (Material == nullptr)
			continue;

		Material->SetScalarParameterValue(TEXT("Gamma"), OutputColorBarrier.GetGamma());
	}
}

void UAGX_CameraSensorComponent::UpdateMaterialParametersFrom(
	const FLensDistortionBrownConradyBarrier* LensDistortionBarrier,
	TArray<TObjectPtr<UMaterialInstanceDynamic>>& OutMaterials)
{
	const bool bHasLensDistortion =
		LensDistortionBarrier != nullptr && LensDistortionBarrier->HasNative();
	const double K1 = bHasLensDistortion ? LensDistortionBarrier->GetK1() : 0.0;
	const double K2 = bHasLensDistortion ? LensDistortionBarrier->GetK2() : 0.0;
	const double K3 = bHasLensDistortion ? LensDistortionBarrier->GetK3() : 0.0;
	const double P1 = bHasLensDistortion ? LensDistortionBarrier->GetP1() : 0.0;
	const double P2 = bHasLensDistortion ? LensDistortionBarrier->GetP2() : 0.0;

	for (auto& Material : OutMaterials)
	{
		if (Material == nullptr)
			continue;

		Material->SetScalarParameterValue(TEXT("LD_K1"), K1);
		Material->SetScalarParameterValue(TEXT("LD_K2"), K2);
		Material->SetScalarParameterValue(TEXT("LD_K3"), K3);
		Material->SetScalarParameterValue(TEXT("LD_P1"), P1);
		Material->SetScalarParameterValue(TEXT("LD_P2"), P2);
	}
}

bool UAGX_CameraSensorComponent::UpdateOutputRenderContextNoParams(
	FCameraOutputRenderContext& OutputRenderContext,
	const FCameraOutputColorBarrier& OutputColorBarrier, bool bLogWarnings)
{
	using namespace AGX_CameraSensorComponent_helpers;

	const auto LogWarning = [this, bLogWarnings](const TCHAR* Message)
	{
		if (!bLogWarnings)
			return;

		UE_LOG(
			LogAGX, Warning,
			TEXT("Camera Sensor Component '%s' in '%s' cannot update Output Render Context: %s"),
			*GetName(), *GetLabelSafe(GetOwner()), Message);
	};

	if (!OutputColorBarrier.HasNative())
	{
		LogWarning(TEXT("the Camera Color Output does not have a native output."));
		return false;
	}

	USceneCaptureComponent2D* CaptureSource = GetCaptureSource();
	if (CaptureSource == nullptr)
	{
		LogWarning(TEXT("there is no valid Scene Capture Component 2D."));
		return false;
	}

	const FIntPoint OutputResolution = OutputColorBarrier.GetResolution();
	if (!FAGX_CameraOutputBase::IsResolutionValid(OutputResolution))
	{
		LogWarning(TEXT("the Camera Color Output resolution is invalid."));
		return false;
	}

	const UAGX_CameraCMOSSensor& CMOSSensor = GetCMOSSensorOrDefault(*this);
	const UAGX_CameraLensSingleElement& Lens = GetCameraLensSingleElementOrDefault(*this);
	float SceneCaptureFOVAngle =
		CalculateHorizontalFOVDegrees(CMOSSensor.GetSize().X, Lens.GetFocalLength());
	FIntPoint SceneResolution = OutputResolution;
	if (!HasCaptureSourceOverride() && MaterialPasses.Num() > 0)
	{
		const FLensDistortionBrownConradyBarrier LensDistortionBarrier =
			GetLensDistortionBrownConradyBarrier(*this);
		const FLensDistortionBrownConradyBarrier* LensDistortionBarrierPtr =
			LensDistortionBarrier.HasNative() ? &LensDistortionBarrier : nullptr;
		CalculateLensDistortionCompensation(
			SceneCaptureFOVAngle, OutputResolution, LensDistortionBarrierPtr, SceneCaptureFOVAngle,
			SceneResolution);
	}

	if (SceneCaptureFOVAngle > 0.0f)
		OutputRenderContext.SceneCaptureFOVAngle = SceneCaptureFOVAngle;
	OutputRenderContext.SceneCaptureResolution = SceneResolution;

	const EAGX_CameraOutputChannelType ChannelType = OutputColorBarrier.GetChannelType();
	const uint8 ChannelCount = OutputColorBarrier.GetChannelCount();
	const TOptional<ETextureRenderTargetFormat> RenderTargetFormat =
		AGX_CameraSensorComponent_helpers::GetRenderTargetFormat(ChannelType, ChannelCount);
	if (!RenderTargetFormat.IsSet())
	{
		LogWarning(TEXT("the Camera Color Output channel type or channel count is unsupported."));
		return false;
	}

	const EPixelFormat PixelFormat =
		GetPixelFormatFromRenderTargetFormat(RenderTargetFormat.GetValue());
	auto EnsureRenderTarget = [this, ChannelType, ChannelCount, RenderTargetFormat,
							   PixelFormat](
								  TObjectPtr<UTextureRenderTarget2D>& RenderTarget,
								  const FIntPoint& Resolution)
	{
		if (RenderTarget == nullptr)
		{
			RenderTarget = CreateRenderTarget(Resolution, ChannelType, ChannelCount);
			return RenderTarget != nullptr;
		}

		if (RenderTarget->SizeX != Resolution.X || RenderTarget->SizeY != Resolution.Y)
			RenderTarget->ResizeTarget(Resolution.X, Resolution.Y);

		if (RenderTarget->GetFormat() != PixelFormat)
		{
			RenderTarget->RenderTargetFormat = RenderTargetFormat.GetValue();
			RenderTarget->InitAutoFormat(RenderTarget->SizeX, RenderTarget->SizeY);
		}

		return true;
	};

	if (HasCaptureSourceOverride())
	{
		OutputRenderContext.SceneRenderTarget = CaptureSource->TextureTarget;
	}
	else
	{
		if (!EnsureRenderTarget(
				OutputRenderContext.SceneRenderTarget,
				OutputRenderContext.SceneCaptureResolution))
		{
			LogWarning(TEXT("failed to create the Scene Render Target."));
			return false;
		}
	}

	if (OutputRenderContext.MaterialInstances.Num() != MaterialPasses.Num())
		OutputRenderContext.MaterialInstances.SetNum(MaterialPasses.Num());

	for (int32 Index = 0; Index < MaterialPasses.Num(); ++Index)
	{
		UMaterialInterface* Material = MaterialPasses[Index].Get();
		TObjectPtr<UMaterialInstanceDynamic>& MaterialInstance =
			OutputRenderContext.MaterialInstances[Index];
		if (Material == nullptr)
		{
			if (MaterialInstance != nullptr)
				MaterialInstance = nullptr;
			continue;
		}

		if (MaterialInstance == nullptr || MaterialInstance->Parent.Get() != Material)
			MaterialInstance = UMaterialInstanceDynamic::Create(Material, this);
	}

	if (OutputRenderContext.RenderTargets.Num() != OutputRenderContext.MaterialInstances.Num())
		OutputRenderContext.RenderTargets.SetNum(OutputRenderContext.MaterialInstances.Num());

	for (int32 Index = 0; Index < OutputRenderContext.MaterialInstances.Num(); ++Index)
	{
		TObjectPtr<UTextureRenderTarget2D>& RenderTarget = OutputRenderContext.RenderTargets[Index];
		if (OutputRenderContext.MaterialInstances[Index] == nullptr)
		{
			if (RenderTarget != nullptr)
				RenderTarget = nullptr;
			continue;
		}

		if (!EnsureRenderTarget(RenderTarget, OutputResolution))
		{
			LogWarning(TEXT("failed to create a Material Pass Render Target."));
			return false;
		}
	}

	return true;
}

bool UAGX_CameraSensorComponent::RequestCapture(const FCameraOutputColorBarrier& OutputColorBarrier)
{
	using namespace AGX_CameraSensorComponent_helpers;

	FCameraBarrier* CameraBarrier = GetNativeAsCamera();
	if (CameraBarrier == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Camera Sensor Component '%s' in '%s' cannot request a capture because it does "
				 "not have a native Camera."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return false;
	}

	if (!OutputColorBarrier.HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Camera Sensor Component '%s' in '%s' cannot request a capture because the "
				 "Camera Color Output does not have a native output."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return false;
	}

	FCameraOutputRenderContext* OutputRenderContext =
		OutputRenderContexts.Find(OutputColorBarrier.GetNativeAddress());
	if (OutputRenderContext == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Camera Sensor Component '%s' in '%s' cannot request a capture because the "
				 "Camera Color Output does not have a render context."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return false;
	}

	if (OutputRenderContext->SceneRenderTarget == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Camera Sensor Component '%s' in '%s' cannot request a capture because the "
				 "Camera Color Output render context does not have a Scene Render Target."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return false;
	}

	if (!HasCaptureSourceOverride())
	{
		USceneCaptureComponent2D* CaptureSource = GetCaptureSource();
		if (CaptureSource == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Camera Sensor Component '%s' in '%s' cannot request a capture because it "
					 "does not have a Scene Capture Component 2D."),
				*GetName(), *GetLabelSafe(GetOwner()));
			return false;
		}

		if (OutputRenderContext->SceneCaptureFOVAngle > 0.0f &&
			!FMath::IsNearlyEqual(
				CaptureSource->FOVAngle, OutputRenderContext->SceneCaptureFOVAngle))
		{
			CaptureSource->FOVAngle = OutputRenderContext->SceneCaptureFOVAngle;
		}
		CaptureSource->TextureTarget = OutputRenderContext->SceneRenderTarget;

		const UAGX_CameraLensSingleElement& Lens = GetCameraLensSingleElementOrDefault(*this);
		if (Lens.GetUseAutofocus())
			UpdateLensFocalDistance(*CaptureSource, Lens);
	}

	FAGX_CameraSensorCaptureDataPtr Slot = OutputRenderContext->CaptureHelper.GetFreeSlot();
	if (!Slot.IsValid()) // No free slots, deny the request.
		return false;

	UTextureRenderTarget2D* FinalRenderTarget =
		RenderMaterialPasses(*OutputRenderContext, OutputColorBarrier);
	if (FinalRenderTarget == nullptr)
		return false; // Slot is still "Free" for future requests.

	const EPixelFormat PixelFormat = FinalRenderTarget->GetFormat();
	if (!AGX_CameraSensorComponent_helpers::IsSupportedReadbackFormat(PixelFormat))
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Camera Sensor Component '%s' in '%s' cannot read back Render Target '%s' "
				 "because pixel format '%s' does not match FColor-sized output."),
			*GetName(), *GetLabelSafe(GetOwner()), *FinalRenderTarget->GetName(),
			GPixelFormats[PixelFormat].Name);
		return false; // Slot is still "Free" for future requests.
	}

	const FIntPoint ImageSize {FinalRenderTarget->SizeX, FinalRenderTarget->SizeY};
	FTextureRenderTargetResource* FinalRenderTargetResource =
		FinalRenderTarget->GameThread_GetRenderTargetResource();

	if (FinalRenderTargetResource == nullptr)
		return false; // Slot is still "Free" for future requests.

	const FString NameBase = GetName();
	Slot->SetState(EAGX_CameraSensorSlotState::CaptureRequested); // We claim the slot.
	Slot->OutputNativeAddress = OutputColorBarrier.GetNativeAddress();
	Slot->ChannelType = OutputColorBarrier.GetChannelType();
	Slot->ChannelCount = OutputColorBarrier.GetChannelCount();
	ENQUEUE_RENDER_COMMAND(AGXCameraCaptureRequest)
	(
		[FinalRenderTargetResource, Slot, NameBase, ImageSize,
		 PixelFormat](FRHICommandListImmediate& RHICmdList)
		{
			const bool bNeedsNewStagingTexture = !Slot->StagingTexture.IsValid() ||
												 Slot->StagingTexture->GetSizeXY() != ImageSize ||
												 Slot->StagingTexture->GetFormat() != PixelFormat;
			if (bNeedsNewStagingTexture)
			{
				const FRHITextureCreateDesc Desc =
					FRHITextureCreateDesc::Create2D(
						*FString::Printf(TEXT("%s CameraCaptureStagingTexture"), *NameBase))
						.SetExtent(ImageSize.X, ImageSize.Y)
						.SetFormat(PixelFormat)
						.SetFlags(ETextureCreateFlags::Shared | ETextureCreateFlags::CPUReadback);

				Slot->StagingTexture = RHICreateTexture(Desc);
			}

			if (!Slot->StagingTexture.IsValid())
			{
				// Fail. Give back the slot for future requests.
				Slot->CopyFence.SafeRelease();
				Slot->SetState(EAGX_CameraSensorSlotState::Free);
				return;
			}

			FRHITexture* SourceTexture = FinalRenderTargetResource->GetRenderTargetTexture();
			if (SourceTexture == nullptr)
			{
				// Fail. Give back the slot for future requests.
				Slot->CopyFence.SafeRelease();
				Slot->SetState(EAGX_CameraSensorSlotState::Free);
				return;
			}

			RHICmdList.Transition(
				FRHITransitionInfo(SourceTexture, ERHIAccess::Unknown, ERHIAccess::CopySrc));
			RHICmdList.CopyTexture(SourceTexture, Slot->StagingTexture, FRHICopyTextureInfo());

			Slot->CopyFence.SafeRelease();
			Slot->CopyFence = RHICreateGPUFence(TEXT("AGXCameraCaptureCopyFence"));
			RHICmdList.WriteGPUFence(Slot->CopyFence);
			Slot->SetState(EAGX_CameraSensorSlotState::AwaitingCopyFence);
		});

	return true;
}

void UAGX_CameraSensorComponent::PollCaptures()
{
	TArray<FAGX_CameraSensorCaptureDataPtr> Slots;
	for (auto& Pair : OutputRenderContexts)
	{
		Slots.Append(Pair.Value.CaptureHelper.GetAwaitingCopyFenceSlots());
	}

	if (Slots.Num() == 0)
		return; // Nothing to do yet.

	for (FAGX_CameraSensorCaptureDataPtr& Slot : Slots)
	{
		if (!Slot.IsValid())
			continue;

		Slot->SetState(EAGX_CameraSensorSlotState::PollCopyFence); // Claim slot for polling.
	}

	ENQUEUE_RENDER_COMMAND(AGXCameraPollCapture)
	(
		[Slots](FRHICommandListImmediate& RHICmdList)
		{
			for (const FAGX_CameraSensorCaptureDataPtr& Slot : Slots)
			{
				if (!Slot.IsValid())
					continue;

				if (Slot->GetState() != EAGX_CameraSensorSlotState::PollCopyFence)
					continue;

				if (!Slot->StagingTexture.IsValid() || !Slot->CopyFence.IsValid())
				{
					// Fail. Give back the slot for future requests.
					Slot->CopyFence.SafeRelease();
					Slot->SetState(EAGX_CameraSensorSlotState::Free);
					continue;
				}

				if (!Slot->CopyFence->Poll())
				{
					// Not yet ready, put slot back to AwaitingCopyFence and it will be processed
					// again next call to this function.
					Slot->SetState(EAGX_CameraSensorSlotState::AwaitingCopyFence);
					continue;
				}

				// At this point the StagingTexture has been written to on the GPU and we are ready
				// to read the pixel data from it.
				void* PixelBuffer = nullptr;
				int32 SurfaceWidth = 0;
				int32 SurfaceHeight = 0;

				// This does not copy bytes into PixelBuffer, it simply assigns the pointer so that
				// we can read off of it from the CPU. This ptr becomes invalid after the
				// UnmapStagingSurface call further down.
				GDynamicRHI->RHIMapStagingSurface(
					Slot->StagingTexture, Slot->CopyFence, PixelBuffer, SurfaceWidth, SurfaceHeight,
					RHICmdList.GetGPUMask().ToIndex());

				if (PixelBuffer == nullptr)
				{
					// Fail. Give back the slot for future requests.
					Slot->CopyFence.SafeRelease();
					Slot->SetState(EAGX_CameraSensorSlotState::Free);
					continue;
				}

				const FIntPoint ImageSize = Slot->StagingTexture->GetSizeXY();
				const int32 LogicalWidth = ImageSize.X;
				const int32 LogicalHeight = ImageSize.Y;
				const EPixelFormat PixelFormat = Slot->StagingTexture->GetFormat();
				const int32 SourceBytesPerPixel = GPixelFormats[PixelFormat].BlockBytes;
				const TOptional<int32> ChannelSize =
					AGX_CameraSensorComponent_helpers::GetChannelSize(Slot->ChannelType);
				const int32 ChannelCount = static_cast<int32>(Slot->ChannelCount);
				if (LogicalWidth <= 0 || LogicalHeight <= 0 || SourceBytesPerPixel <= 0 ||
					SurfaceWidth < LogicalWidth || SurfaceHeight < LogicalHeight ||
					!ChannelSize.IsSet() || ChannelCount < 1 || ChannelCount > 4)
				{
					RHICmdList.UnmapStagingSurface(Slot->StagingTexture);
					Slot->CopyFence.SafeRelease();
					Slot->SetState(EAGX_CameraSensorSlotState::Free);
					continue;
				}

				const int32 DestinationBytesPerPixel = ChannelSize.GetValue() * ChannelCount;
				if (DestinationBytesPerPixel > SourceBytesPerPixel)
				{
					RHICmdList.UnmapStagingSurface(Slot->StagingTexture);
					Slot->CopyFence.SafeRelease();
					Slot->SetState(EAGX_CameraSensorSlotState::Free);
					continue;
				}

				{
					FCameraOutputRawDataWriteAccess OutputRawData =
						FCameraBackendBarrier::GetInstance().LockOutputRawDataForWrite(
							Slot->OutputNativeAddress);
					if (OutputRawData.Get() == nullptr)
					{
						RHICmdList.UnmapStagingSurface(Slot->StagingTexture);
						Slot->CopyFence.SafeRelease();
						Slot->SetState(EAGX_CameraSensorSlotState::Free);
						continue;
					}

					OutputRawData->Resolution = ImageSize;
					OutputRawData->PixelFormat = PixelFormat;
					OutputRawData->IsUnread = true;

					const int32 SourcePitch = SurfaceWidth * SourceBytesPerPixel;
					const int32 DestinationPitch = LogicalWidth * DestinationBytesPerPixel;
					const int32 NumBytes = DestinationPitch * LogicalHeight;
					OutputRawData->RawData.SetNumUninitialized(NumBytes, EAllowShrinking::No);
					if (SurfaceWidth == LogicalWidth &&
						SourceBytesPerPixel == DestinationBytesPerPixel)
					{
						FMemory::Memcpy(OutputRawData->RawData.GetData(), PixelBuffer, NumBytes);
					}
					else
					{
						const uint8* SourceRow = static_cast<const uint8*>(PixelBuffer);
						uint8* DestinationRow = OutputRawData->RawData.GetData();
						for (int32 Row = 0; Row < LogicalHeight; ++Row)
						{
							if (SourceBytesPerPixel == DestinationBytesPerPixel)
							{
								FMemory::Memcpy(DestinationRow, SourceRow, DestinationPitch);
							}
							else
							{
								const uint8* SourcePixel = SourceRow;
								uint8* DestinationPixel = DestinationRow;
								for (int32 Column = 0; Column < LogicalWidth; ++Column)
								{
									FMemory::Memcpy(
										DestinationPixel, SourcePixel, DestinationBytesPerPixel);
									SourcePixel += SourceBytesPerPixel;
									DestinationPixel += DestinationBytesPerPixel;
								}
							}

							SourceRow += SourcePitch;
							DestinationRow += DestinationPitch;
						}
					}
				}

				RHICmdList.UnmapStagingSurface(Slot->StagingTexture);
				Slot->CopyFence.SafeRelease();

				// We are done, give back the slot.
				Slot->SetState(EAGX_CameraSensorSlotState::Free);
			}
		});
}

FSensorBarrier* UAGX_CameraSensorComponent::CreateNativeImpl()
{
	Super::CreateNativeImpl();

	AGX_CHECK(!HasNative());
	if (HasNative())
		return GetNativeAsCamera();

	auto CameraBackend = UAGX_CameraBackend::GetFrom(this);
	AGX_CHECK(CameraBackend != nullptr);
	if (CameraBackend == nullptr)
		return nullptr;

	auto CameraBackendBarrier = CameraBackend->GetOrCreateNative();
	if (CameraBackendBarrier == nullptr || !CameraBackendBarrier->HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("UAGX_CameraSensorComponent::CreateNativeImpl called on Camera Sensor Component "
				 "'%s' in '%s' but the Camera Backend does not have a valid Native Object. Native "
				 "Camera Sensor will not be created."),
			*GetName(), *GetLabelSafe(GetOwner()));
		return nullptr;
	}

	FCameraBarrier* CameraBarrier = static_cast<FCameraBarrier*>(NativeBarrier.Get());
	if (CameraBarrier == nullptr)
		return nullptr;

	UpdateCameraPhotoDetector();
	UpdateCameraLens();
	FCameraLensBarrier* LensBarrier =
		CameraLens != nullptr && CameraLens->HasNative() ? CameraLens->GetNative() : nullptr;
	FCameraPhotodetectorBarrier* PhotoDetectorBarrier =
		PhotoDetector != nullptr && PhotoDetector->HasNative() ? PhotoDetector->GetNative()
															   : nullptr;

	CameraBarrier->AllocateNative(GetComponentTransform(), LensBarrier, PhotoDetectorBarrier);
	SetupCameraBackendPropagator();
	if (HasNative())
		UpdateNativeProperties();

	return CameraBarrier;
}

void UAGX_CameraSensorComponent::CopyFrom(
	const FSensorBarrier& Barrier, FAGX_ImportContext* Context)
{
	Super::CopyFrom(Barrier, Context);

	AGX_CHECK(!Context->Sensors.Contains(ImportGuid));
	Context->Sensors.Add(ImportGuid, this);

	// TODO: copy settings here.
}

void UAGX_CameraSensorComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GIsReconstructingBlueprintInstances)
		return;

	if (!HasNative())
		CreateNativeImpl();

	if (HasNative())
	{
		SetupSceneCapture();

		if (auto Se = UAGX_SensorEnvironmentSubsystem::GetFrom(this))
		{
			Se->AddCamera(this);
		}
	}
}

void UAGX_CameraSensorComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	if (!GIsReconstructingBlueprintInstances && HasNative() &&
		Reason != EEndPlayReason::EndPlayInEditor && Reason != EEndPlayReason::Quit &&
		Reason != EEndPlayReason::LevelTransition)
	{
		if (auto Se = UAGX_SensorEnvironmentSubsystem::GetFrom(this))
		{
			Se->RemoveCamera(this);
		}
	}

	Super::EndPlay(Reason);
}

void UAGX_CameraSensorComponent::PostApplyToComponent()
{
	using namespace AGX_CameraSensorComponent_helpers;

	Super::PostApplyToComponent();

	if (GIsReconstructingBlueprintInstances && HasNative() && GetWorld() &&
		GetWorld()->IsGameWorld())
	{
		// Dynamic Components are not carried over when a Blueprint instance is reconstructed
		// during Play, so recreate the runtime Scene Capture Component on the new instance.
		FCameraBarrier* CameraBarrier = GetNativeAsCamera();
		if (CameraBarrier == nullptr)
			return;

		if (auto Se = UAGX_SensorEnvironmentSubsystem::GetFrom(this))
		{
			Se->AddCamera(this);
		}

		CameraBarrier->RegisterWithBackend();
		SetupCameraBackendPropagator();
		SetupSceneCapture();

		OutputRenderContexts.Empty();
		const FLensDistortionBrownConradyBarrier LensDistortionBarrier =
			GetLensDistortionBrownConradyBarrier(*this);
		const FLensDistortionBrownConradyBarrier* LensDistortionBarrierPtr =
			LensDistortionBarrier.HasNative() ? &LensDistortionBarrier : nullptr;
		TArray<FCameraOutputBarrier> OutputBarriers = CameraBarrier->GetOutputs();
		for (FCameraOutputBarrier& OutputBarrier : OutputBarriers)
		{
			OutputBarrier.RegisterWithBackend(*CameraBarrier);
			if (!FCameraOutputColorBarrier::IsColorOutput(OutputBarrier))
				continue;

			FCameraOutputColorBarrier OutputColorBarrier =
				FCameraOutputColorBarrier::CreateFrom(OutputBarrier);
			FCameraOutputRenderContext* OutputRenderContext =
				UpdateOutputCaptureSettings(OutputColorBarrier);
			if (OutputRenderContext == nullptr)
				continue;

			UpdateMaterialParametersFrom(
				OutputColorBarrier, OutputRenderContext->MaterialInstances);
			UpdateMaterialParametersFrom(
				LensDistortionBarrierPtr, OutputRenderContext->MaterialInstances);
		}
	}
}

void UAGX_CameraSensorComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	PollCaptures();
}

void UAGX_CameraSensorComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);

	if (OwnedCaptureComponent2D != nullptr)
	{
		OwnedCaptureComponent2D->DestroyComponent();
		OwnedCaptureComponent2D = nullptr;
	}

	OutputRenderContexts.Empty();
}

#if WITH_EDITOR
bool UAGX_CameraSensorComponent::CanEditChange(const FProperty* InProperty) const
{
	const bool SuperCanEditChange = Super::CanEditChange(InProperty);
	if (!SuperCanEditChange)
		return false;

	if (InProperty == nullptr)
		return SuperCanEditChange;

	const bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
	if (bIsPlaying)
	{
		static const TArray<FName> PropertiesNotEditableDuringPlay {
			GET_MEMBER_NAME_CHECKED(ThisClass, PhotoDetector),
			GET_MEMBER_NAME_CHECKED(ThisClass, CameraLens)};

		if (PropertiesNotEditableDuringPlay.Contains(InProperty->GetFName()))
			return false;
	}

	return SuperCanEditChange;
}

void UAGX_CameraSensorComponent::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);

	Super::PostEditChangeChainProperty(Event);
}

void UAGX_CameraSensorComponent::PostInitProperties()
{
	Super::PostInitProperties();
	InitPropertyDispatcher();
}
#endif

void UAGX_CameraSensorComponent::OnRegister()
{
	Super::OnRegister();

	// On Register is called after object initialization has completed, so it is safe to set the
	// local scope used by Component References.
	AGX_CameraSensorComponent_helpers::SetLocalScope(*this);
}

FCameraBarrier* UAGX_CameraSensorComponent::GetNativeAsCamera()
{
	if (!HasNative())
		return nullptr;

	return static_cast<FCameraBarrier*>(NativeBarrier.Get());
}

const FCameraBarrier* UAGX_CameraSensorComponent::GetNativeAsCamera() const
{
	if (!HasNative())
		return nullptr;

	return static_cast<const FCameraBarrier*>(NativeBarrier.Get());
}

void UAGX_CameraSensorComponent::MarkOutputAsRead()
{
	if (bOpenPLXImported)
		return; // OpenPLX imported sensors outputs are handled with OpenPLX signals.

	if (HasNative())
		GetNativeAsCamera()->MarkOutputAsRead();
}

void UAGX_CameraSensorComponent::UpdateNativeProperties()
{
	AGX_CHECK(HasNative());

	Super::UpdateNativeProperties();
	UpdateNativeTransform();
}

void UAGX_CameraSensorComponent::SetupSceneCapture()
{
	if (HasCaptureSourceOverride())
	{
		if (OwnedCaptureComponent2D != nullptr)
		{
			OwnedCaptureComponent2D->DestroyComponent();
			OwnedCaptureComponent2D = nullptr;
		}
		return;
	}

	if (OwnedCaptureComponent2D != nullptr)
		return;

	AActor* Owner = GetOwner();
	if (Owner == nullptr)
		return;

	UWorld* World = GetWorld();
	if (World == nullptr)
		return;

	// No CaptureSourceOverride set by the user, create a USceneCaptureComponent2D that we own
	// completely and use that when rendering.

	OwnedCaptureComponent2D =
		NewObject<USceneCaptureComponent2D>(this, FName(TEXT("SceneCaptureComponent2D")));
	OwnedCaptureComponent2D->CreationMethod = EComponentCreationMethod::Native;
	OwnedCaptureComponent2D->SetCanEverAffectNavigation(false);
	OwnedCaptureComponent2D->PrimitiveRenderMode =
		ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
	OwnedCaptureComponent2D->CaptureSource = ESceneCaptureSource::SCS_FinalColorHDR;
	OwnedCaptureComponent2D->bCaptureEveryFrame = false;
	OwnedCaptureComponent2D->bCaptureOnMovement = false;
	OwnedCaptureComponent2D->bAlwaysPersistRenderingState = true;
	OwnedCaptureComponent2D->PostProcessBlendWeight = 1.0f;
	OwnedCaptureComponent2D->PostProcessSettings.bOverride_CameraISO = true;
	OwnedCaptureComponent2D->PostProcessSettings.bOverride_AutoExposureMethod = true;
	OwnedCaptureComponent2D->PostProcessSettings.bOverride_AutoExposureBias = true;
	OwnedCaptureComponent2D->PostProcessSettings.bOverride_AutoExposureMaxBrightness = true;
	OwnedCaptureComponent2D->PostProcessSettings.bOverride_DepthOfFieldFstop = true;
	OwnedCaptureComponent2D->PostProcessSettings.bOverride_DepthOfFieldSensorWidth = true;
	OwnedCaptureComponent2D->PostProcessSettings.bOverride_DepthOfFieldFocalDistance = true;
	OwnedCaptureComponent2D->RegisterComponent();
	OwnedCaptureComponent2D->AttachToComponent(
		this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
}

void UAGX_CameraSensorComponent::SetupCameraBackendPropagator()
{
	CameraBackendPropagator.SetCameraSensor(this);

	if (FCameraBarrier* CameraBarrier = GetNativeAsCamera())
		CameraBarrier->SetBackendPropagator(&CameraBackendPropagator);
}

void UAGX_CameraSensorComponent::UpdateCameraPhotoDetector()
{
	if (PhotoDetector == nullptr)
		return;

	UWorld* World = GetWorld();
	UAGX_CameraPhotodetectorBase* Instance = PhotoDetector->GetOrCreateInstance(World);
	if (Instance == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Camera Sensor Component '%s' in '%s' failed to create a PhotoDetector instance "
				 "from '%s'. Default AGX CMOS Sensor will be used."),
			*GetName(), *GetLabelSafe(GetOwner()), *PhotoDetector->GetName());
		return;
	}

	PhotoDetector = Instance;

	FCameraPhotodetectorBarrier* Barrier = PhotoDetector->GetOrCreateNative();
	if (Barrier == nullptr || !Barrier->HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Camera Sensor Component '%s' in '%s' failed to create a native PhotoDetector "
				 "from instance '%s'. Default AGX CMOS Sensor will be used."),
			*GetName(), *GetLabelSafe(GetOwner()), *PhotoDetector->GetName());
	}
}

void UAGX_CameraSensorComponent::UpdateCameraLens()
{
	if (CameraLens == nullptr)
		return;

	UWorld* World = GetWorld();
	UAGX_CameraLensBase* Instance = CameraLens->GetOrCreateInstance(World);
	if (Instance == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Camera Sensor Component '%s' in '%s' failed to create a CameraLens instance "
				 "from '%s'. Default AGX single element lens will be used."),
			*GetName(), *GetLabelSafe(GetOwner()), *CameraLens->GetName());
		return;
	}

	CameraLens = Instance;

	FCameraLensBarrier* Barrier = CameraLens->GetOrCreateNative();
	if (Barrier == nullptr || !Barrier->HasNative())
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Camera Sensor Component '%s' in '%s' failed to create a native CameraLens "
				 "from instance '%s'. Default AGX single element lens will be used."),
			*GetName(), *GetLabelSafe(GetOwner()), *CameraLens->GetName());
	}
}

UTextureRenderTarget2D* UAGX_CameraSensorComponent::CreateRenderTarget(
	const FIntPoint& InResolution, EAGX_CameraOutputChannelType ChannelType, uint8 ChannelCount)
{
	const TOptional<ETextureRenderTargetFormat> RenderTargetFormat =
		AGX_CameraSensorComponent_helpers::GetRenderTargetFormat(ChannelType, ChannelCount);
	if (!RenderTargetFormat.IsSet())
		return nullptr;

	UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>(this);
	RenderTarget->bGPUSharedFlag = true;
	RenderTarget->SRGB = false;
	RenderTarget->RenderTargetFormat = RenderTargetFormat.GetValue();
	RenderTarget->InitAutoFormat(InResolution.X, InResolution.Y);
	return RenderTarget;
}

#if WITH_EDITOR
void UAGX_CameraSensorComponent::InitPropertyDispatcher()
{
	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
		return;

	PropertyDispatcher.Add(
		AGX_MEMBER_NAME(CaptureSourceOverride),
		[](ThisClass* This)
		{
			This->SetCaptureSourceOverride(
				This->CaptureSourceOverride.GetSceneCaptureComponent2D());
		});
}
#endif

/// Camera Backend Callbacks.

void UAGX_CameraSensorComponent::OnBackendSetCameraLensSingleElement(
	const FCameraLensSingleElementBarrier& LensBarrier)
{
	using namespace AGX_CameraSensorComponent_helpers;

	if (HasCaptureSourceOverride())
		return; // Never modify users camera.

	USceneCaptureComponent2D* SceneCapture = OwnedCaptureComponent2D.Get();
	AGX_CHECK(SceneCapture != nullptr);
	if (SceneCapture == nullptr)
		return;

	UpdateAllOutputCaptureSettings();

	FPostProcessSettings& PostProcessSettings = SceneCapture->PostProcessSettings;
	PostProcessSettings.DepthOfFieldFstop = static_cast<float>(LensBarrier.GetFStop());

	UpdateLensFocalDistance(
		*SceneCapture, LensBarrier.GetUseAutofocus(), LensBarrier.GetMinimumFocusDistance(),
		LensBarrier.GetFocusDistance());
}

void UAGX_CameraSensorComponent::OnBackendSetCameraCMOSSensor(
	const FCameraCMOSSensorBarrier& CMOSBarrier)
{
	if (HasCaptureSourceOverride())
		return; // Never modify users camera.

	USceneCaptureComponent2D* SceneCapture = OwnedCaptureComponent2D.Get();
	AGX_CHECK(SceneCapture != nullptr);
	if (SceneCapture == nullptr)
		return;

	UpdateAllOutputCaptureSettings();

	FPostProcessSettings& PostProcessSettings = SceneCapture->PostProcessSettings;
	const bool AutoExposure = CMOSBarrier.GetUseAutoExposure();
	PostProcessSettings.AutoExposureMethod =
		AutoExposure ? EAutoExposureMethod::AEM_Histogram : EAutoExposureMethod::AEM_Manual;

	if (AutoExposure)
		PostProcessSettings.AutoExposureBias = 1;
	else
		PostProcessSettings.AutoExposureBias = CMOSBarrier.GetExposureCompensation();

	PostProcessSettings.AutoExposureMaxBrightness = CMOSBarrier.GetDynamicRange();
	PostProcessSettings.CameraISO = CMOSBarrier.GetISO();
	PostProcessSettings.DepthOfFieldSensorWidth =
		static_cast<float>(CMOSBarrier.GetSize().X * /*to mm*/ 10.0);
}

void UAGX_CameraSensorComponent::OnBackendSetCameraLensDistortionNone()
{
	if (CameraLens != nullptr)
		CameraLens->LensDistortion = nullptr;

	UpdateAllOutputCaptureSettings();

	for (auto& OutputRenderContextPair : OutputRenderContexts)
	{
		UpdateMaterialParametersFrom(nullptr, OutputRenderContextPair.Value.MaterialInstances);
	}
}

void UAGX_CameraSensorComponent::OnBackendSetCameraLensDistortionBrownConrady(
	const FLensDistortionBrownConradyBarrier& LensDistortionBarrier)
{
	UpdateAllOutputCaptureSettings();

	for (auto& OutputRenderContextPair : OutputRenderContexts)
	{
		UpdateMaterialParametersFrom(
			&LensDistortionBarrier, OutputRenderContextPair.Value.MaterialInstances);
	}
}

void UAGX_CameraSensorComponent::OnBackendSetCameraColorOutput(
	const FCameraOutputColorBarrier& OutputColorBarrier)
{
	FCameraOutputRenderContext* OutputRenderContext =
		UpdateOutputCaptureSettings(OutputColorBarrier);
	if (OutputRenderContext == nullptr)
		return;

	UpdateMaterialParametersFrom(OutputColorBarrier, OutputRenderContext->MaterialInstances);
}

void UAGX_CameraSensorComponent::OnBackendRequestCapture(const FCameraOutputBarrier& OutputBarrier)
{
	if (!FCameraOutputColorBarrier::IsColorOutput(OutputBarrier))
		return;

	const FCameraOutputColorBarrier& ColorOutputBarrier =
		static_cast<const FCameraOutputColorBarrier&>(OutputBarrier);
	RequestCapture(ColorOutputBarrier);
}
