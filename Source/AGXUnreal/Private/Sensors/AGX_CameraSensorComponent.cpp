// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_CameraSensorComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "Import/AGX_ImportContext.h"
#include "Sensors/AGX_CameraBackend.h"
#include "Sensors/AGX_CameraLensBase.h"
#include "Sensors/AGX_CameraOutputBase.h"
#include "Sensors/AGX_CameraPhotodetectorBase.h"
#include "Sensors/AGX_SensorEnvironmentSubsystem.h"
#include "Sensors/CameraBackendBarrier.h"
#include "Sensors/CameraBackendParameters.h"
#include "Sensors/CameraBarrier.h"
#include "Sensors/CameraLensBarrier.h"
#include "Sensors/CameraOutputBarrier.h"
#include "Sensors/CameraOutputColorBarrier.h"
#include "Sensors/CameraPhotodetectorBarrier.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "CanvasItem.h"
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

	static const TCHAR* CameraPass1AssetPath =
		TEXT("Material'/AGXUnreal/Sensor/Camera/Materials/MI_AGX_Camera_Pass1.MI_AGX_Camera_Pass1'");
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

	bool IsSupportedReadbackFormat(EPixelFormat PixelFormat)
	{
		return GPixelFormats[PixelFormat].BlockBytes == sizeof(FColor);
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
		auto RenderContext = GetOrCreateOutputRenderContext(ColorOutputNative);
		UpdateOutputRenderContextNoParams(*RenderContext, ColorOutputNative);
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
	AGX_CHECK(OutputRenderContext.MaterialInstances.Num() == OutputRenderContext.RenderTargets.Num());
	for (int32 Index = 0; Index < OutputRenderContext.MaterialInstances.Num(); ++Index)
	{
		UMaterialInstanceDynamic* MaterialInstance =
			OutputRenderContext.MaterialInstances[Index];
		if (MaterialInstance == nullptr)
			continue;

		AGX_CHECK(OutputRenderContext.RenderTargets.IsValidIndex(Index));
		if (!OutputRenderContext.RenderTargets.IsValidIndex(Index) ||
			OutputRenderContext.RenderTargets[Index] == nullptr)
			continue;

		UTextureRenderTarget2D* OutputRenderTarget =
			OutputRenderContext.RenderTargets[Index].Get();
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

UAGX_CameraSensorComponent::FCameraOutputRenderContext*
UAGX_CameraSensorComponent::GetOrCreateOutputRenderContext(
	const FCameraOutputColorBarrier& OutputColorBarrier)
{
	if (!OutputColorBarrier.HasNative())
		return nullptr;

	return &OutputRenderContexts.FindOrAdd(OutputColorBarrier.GetNativeAddress());
}

void UAGX_CameraSensorComponent::UpdateMaterialParameters(
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

bool UAGX_CameraSensorComponent::UpdateOutputRenderContextNoParams(
	FCameraOutputRenderContext& OutputRenderContext,
	const FCameraOutputColorBarrier& OutputColorBarrier)
{
	if (!OutputColorBarrier.HasNative())
		return false;

	USceneCaptureComponent2D* CaptureSource = GetCaptureSource();
	if (CaptureSource == nullptr)
		return false;

	const FIntPoint Resolution = OutputColorBarrier.GetResolution();
	if (!IsResolutionValid(Resolution))
		return false;

	if (HasCaptureSourceOverride())
	{
		OutputRenderContext.SceneRenderTarget = CaptureSource->TextureTarget;
	}
	else
	{
		if (!IsRenderTargetUpToDate(OutputRenderContext.SceneRenderTarget.Get(), Resolution))
			OutputRenderContext.SceneRenderTarget = CreateRenderTarget(Resolution);
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

		if (!IsRenderTargetUpToDate(RenderTarget.Get(), Resolution))
			RenderTarget = CreateRenderTarget(Resolution);
	}

	return true;
}

bool UAGX_CameraSensorComponent::RequestCapture(
	const FCameraOutputColorBarrier& OutputColorBarrier)
{
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
		GetOrCreateOutputRenderContext(OutputColorBarrier);

	if (!UpdateOutputRenderContextNoParams(*OutputRenderContext, OutputColorBarrier))
		return false;

	if (!HasCaptureSourceOverride())
		GetCaptureSource()->TextureTarget = OutputRenderContext->SceneRenderTarget;

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
	ENQUEUE_RENDER_COMMAND(AGXCameraCaptureRequest)
	(
		// TODO, some of these captures are note safe for Blueprint reconstruction.
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
		// TODO: 'this' capture is not safe from Blueprint reconstruction.
		[Slots, this](FRHICommandListImmediate& RHICmdList)
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
				const int32 BytesPerPixel = GPixelFormats[PixelFormat].BlockBytes;
				if (LogicalWidth <= 0 || LogicalHeight <= 0 || BytesPerPixel <= 0 ||
					SurfaceWidth < LogicalWidth || SurfaceHeight < LogicalHeight)
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

					const int32 SourcePitch = SurfaceWidth * BytesPerPixel;
					const int32 DestinationPitch = LogicalWidth * BytesPerPixel;
					const int32 NumBytes = DestinationPitch * LogicalHeight;
					OutputRawData->RawData.SetNumUninitialized(NumBytes, EAllowShrinking::No);
					if (SurfaceWidth == LogicalWidth)
					{
						FMemory::Memcpy(OutputRawData->RawData.GetData(), PixelBuffer, NumBytes);
					}
					else
					{
						const uint8* SourceRow = static_cast<const uint8*>(PixelBuffer);
						uint8* DestinationRow = OutputRawData->RawData.GetData();
						for (int32 Row = 0; Row < LogicalHeight; ++Row)
						{
							FMemory::Memcpy(DestinationRow, SourceRow, DestinationPitch);
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

	AGX_CHECK(!Context->Sensors->Contains(ImportGuid));
	Context->Sensors->Add(ImportGuid, this);
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
	Super::PostApplyToComponent();

	if (GIsReconstructingBlueprintInstances && HasNative() && GetWorld() &&
		GetWorld()->IsGameWorld())
	{
		// Dynamic Components are not carried over when a Blueprint instance is reconstructed
		// during Play, so recreate the runtime Scene Capture Component on the new instance.
		GetNativeAsCamera()->RegisterWithBackend();
		SetupCameraBackendPropagator();
		SetupSceneCapture();

		// TODO: ensure OutputRenderContexts state here!
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
	OwnedCaptureComponent2D->RegisterComponent();
	OwnedCaptureComponent2D->AttachToComponent(
		this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	OwnedCaptureComponent2D->SetCanEverAffectNavigation(false);
	OwnedCaptureComponent2D->bCaptureEveryFrame = false;
	OwnedCaptureComponent2D->bCaptureOnMovement = false;
	OwnedCaptureComponent2D->bAlwaysPersistRenderingState = true;
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
	const FIntPoint& InResolution)
{
	UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>(this);
	RenderTarget->bGPUSharedFlag = true;
	RenderTarget->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
	RenderTarget->TargetGamma = 2.2f;
	RenderTarget->InitAutoFormat(InResolution.X, InResolution.Y);
	return RenderTarget;
}

bool UAGX_CameraSensorComponent::IsRenderTargetUpToDate(
	const UTextureRenderTarget2D* RenderTarget, const FIntPoint& InResolution) const
{
	return RenderTarget != nullptr && RenderTarget->SizeX == InResolution.X &&
		   RenderTarget->SizeY == InResolution.Y;
}

bool UAGX_CameraSensorComponent::IsResolutionValid(const FIntPoint& InResolution)
{
	return InResolution.X >= 1 && InResolution.Y >= 1;
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
	const FCameraLensSingleElementParameters& Parameters)
{
	using namespace AGX_CameraSensorComponent_helpers;

	if (HasCaptureSourceOverride())
		return; // Never modify users camera.

	USceneCaptureComponent2D* SceneCapture = OwnedCaptureComponent2D.Get();
	AGX_CHECK(SceneCapture != nullptr);
	if (SceneCapture == nullptr)
		return;

	constexpr double DefaultCMOSSensorWidth {
		0.27288}; // TODO: read from CameraCMOSSensor asset instead!
	const float FOVAngle =
		CalculateHorizontalFOVDegrees(DefaultCMOSSensorWidth, Parameters.focalLength);
	if (FOVAngle > 0.0f)
		SceneCapture->FOVAngle = FOVAngle;

	// SceneCapture->PostProcessBlendWeight = 1.0f;
	// SceneCapture->PostProcessSettings.bOverride_DepthOfFieldEnabled = true;
	// SceneCapture->PostProcessSettings.DepthOfFieldEnabled = true;
	// SceneCapture->PostProcessSettings.bOverride_DepthOfFieldFstop = true;
	// SceneCapture->PostProcessSettings.DepthOfFieldFstop = static_cast<float>(Parameters.fStop);
	// SceneCapture->PostProcessSettings.bOverride_DepthOfFieldSensorWidth = true;
	// SceneCapture->PostProcessSettings.DepthOfFieldSensorWidth =
	//	AGX_CameraSensorComponent_helpers::DefaultCMOSSensorWidthMillimeters;

	// if (Parameters.autofocus)
	//{
	//	SceneCapture->PostProcessSettings.bOverride_DepthOfFieldFocalDistance = false;
	// }
	// else
	//{
	//	SceneCapture->PostProcessSettings.bOverride_DepthOfFieldFocalDistance = true;
	//	SceneCapture->PostProcessSettings.DepthOfFieldFocalDistance =
	//		static_cast<float>(Parameters.focus.distance);
	// }
}

void UAGX_CameraSensorComponent::OnBackendSetCameraColorOutput(
	const FCameraOutputColorBarrier& OutputColorBarrier)
{
	FCameraOutputRenderContext* OutputRenderContext =
		GetOrCreateOutputRenderContext(OutputColorBarrier);
	if (OutputRenderContext == nullptr)
		return;

	UpdateOutputRenderContextNoParams(*OutputRenderContext, OutputColorBarrier);
	UpdateMaterialParameters(OutputColorBarrier, OutputRenderContext->MaterialInstances);
}

void UAGX_CameraSensorComponent::OnBackendRequestCapture(const FCameraOutputBarrier& OutputBarrier)
{
	if (!FCameraOutputColorBarrier::IsColorOutput(OutputBarrier))
		return;

	const FCameraOutputColorBarrier& ColorOutputBarrier =
		static_cast<const FCameraOutputColorBarrier&>(OutputBarrier);
	RequestCapture(ColorOutputBarrier);
}
