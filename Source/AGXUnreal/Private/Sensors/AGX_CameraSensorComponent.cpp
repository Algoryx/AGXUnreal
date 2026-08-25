// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_CameraSensorComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
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

void UAGX_CameraSensorComponent::SetResolution(FIntPoint InResolution)
{
	if (!IsResolutionValid(InResolution))
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Camera Sensor Component '%s' in '%s' received invalid Resolution '%s'. "
				 "Resolution will not be changed."),
			*GetName(), *GetLabelSafe(GetOwner()), *InResolution.ToString());
		return;
	}

	Resolution = InResolution;
	SetupRenderPasses();
}

void UAGX_CameraSensorComponent::SetMaterialInputTextureParameterName(FName InParameterName)
{
	MaterialInputTextureParameterName = InParameterName;
	SetupRenderPasses();
}

void UAGX_CameraSensorComponent::AddMaterialPass(UMaterialInterface* Material)
{
	MaterialPasses.Add(Material);
	SetupRenderPasses();
}

bool UAGX_CameraSensorComponent::SetMaterialPass(int32 Index, UMaterialInterface* Material)
{
	if (!MaterialPasses.IsValidIndex(Index))
		return false;

	MaterialPasses[Index] = Material;
	SetupRenderPasses();
	return true;
}

bool UAGX_CameraSensorComponent::RemoveMaterialPass(UMaterialInterface* Material)
{
	const int32 NumRemoved =
		MaterialPasses.RemoveAll([Material](const TObjectPtr<UMaterialInterface>& ExistingMaterial)
								 { return ExistingMaterial.Get() == Material; });
	if (NumRemoved == 0)
		return false;

	SetupRenderPasses();
	return true;
}

bool UAGX_CameraSensorComponent::RemoveMaterialPassAt(int32 Index)
{
	if (!MaterialPasses.IsValidIndex(Index))
		return false;

	MaterialPasses.RemoveAt(Index);
	SetupRenderPasses();
	return true;
}

void UAGX_CameraSensorComponent::ClearMaterialPasses()
{
	MaterialPasses.Empty();
	SetupRenderPasses();
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
	return true;
}

void UAGX_CameraSensorComponent::SetCaptureSourceOverride(
	USceneCaptureComponent2D* InCaptureSourceOverride)
{
	AGX_CameraSensorComponent_helpers::SetLocalScope(*this);
	CaptureSourceOverride.SetComponent(InCaptureSourceOverride);
	SceneRenderTarget = nullptr;
	RenderTargets.Empty();
	SetupSceneCapture();
	SetupRenderPasses();
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

UTextureRenderTarget2D* UAGX_CameraSensorComponent::RenderCameraPipeline()
{
	USceneCaptureComponent2D* CaptureSource = GetCaptureSource();
	if (CaptureSource == nullptr)
		return nullptr;

	EnsureRenderTargets();
	if (SceneRenderTarget == nullptr)
		return nullptr;

	if (!CaptureSource->bCaptureEveryFrame)
		CaptureSource->CaptureScene();

	UTexture* InputTexture = SceneRenderTarget;
	UTextureRenderTarget2D* FinalRenderTarget = SceneRenderTarget;
	for (int32 Index = 0; Index < MaterialInstances.Num(); ++Index)
	{
		UMaterialInstanceDynamic* MaterialInstance = MaterialInstances[Index];
		if (MaterialInstance == nullptr)
			continue;

		if (!RenderTargets.IsValidIndex(Index) || RenderTargets[Index] == nullptr)
			continue;

		UTextureRenderTarget2D* OutputRenderTarget = RenderTargets[Index].Get();
		MaterialInstance->SetTextureParameterValue(MaterialInputTextureParameterName, InputTexture);

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

UTextureRenderTarget2D* UAGX_CameraSensorComponent::GetOutputRenderTarget() const
{
	for (int32 Index = RenderTargets.Num() - 1; Index >= 0; --Index)
	{
		if (RenderTargets[Index] != nullptr)
			return RenderTargets[Index].Get();
	}

	return SceneRenderTarget.Get();
}

bool UAGX_CameraSensorComponent::RequestCapture()
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

	FAGX_CameraSensorCaptureData* Slot = CaptureHelper.GetFreeSlot();
	if (Slot == nullptr) // No free slots, deny the request.
		return false;

	UTextureRenderTarget2D* FinalRenderTarget = RenderCameraPipeline();
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
	ENQUEUE_RENDER_COMMAND(AGXCameraCaptureRequest)
	(
		// TODO, some of these captures are note safe for Blueprint reconstruction.
		[FinalRenderTargetResource, Slot, NameBase, ImageSize,
		 PixelFormat](FRHICommandListImmediate& RHICmdList)
		{
			// TODO: if size/format can change, we need to verify those also, not just IsValid.
			if (!Slot->StagingTexture.IsValid())
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

void UAGX_CameraSensorComponent::PollCapture()
{
	TArray<FAGX_CameraSensorCaptureData*> Slots = CaptureHelper.GetAwaitingCopyFenceSlots();
	if (Slots.Num() == 0)
		return; // Nothing to do yet.

	for (auto Slot : Slots)
		Slot->SetState(EAGX_CameraSensorSlotState::PollCopyFence); // Claim slot for polling.

	ENQUEUE_RENDER_COMMAND(AGXCameraPollCapture)
	(
		// TODO: 'this' capture is not safe from Blueprint reconstruction.
		[Slots, this](FRHICommandListImmediate& RHICmdList)
		{
			for (auto Slot : Slots)
			{
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

				if (PixelBuffer == nullptr) // TODO: if size can change during runtime, verify size.
				{
					// Fail. Give back the slot for future requests.
					Slot->CopyFence.SafeRelease();
					Slot->SetState(EAGX_CameraSensorSlotState::Free);
					continue;
				}

				const int32 NumPixels = SurfaceWidth * SurfaceHeight;

				// TODO: unsafe write from renderthread.
				FCameraLatestImage& LatestImage =
					static_cast<FCameraBarrier*>(NativeBarrier.Get())->LatestImage;
				LatestImage.Resolution = {SurfaceWidth, SurfaceHeight};
				LatestImage.Pixels.SetNumUninitialized(NumPixels, false);

				// Todo: ensure correct width/height if they may differ between capture and source.
				const int32 SourcePitch = SurfaceWidth * sizeof(FColor);
				const int32 DestinationPitch = SurfaceWidth * sizeof(FColor);
				const uint8* SourceRow = static_cast<const uint8*>(PixelBuffer);
				uint8* DestinationRow = reinterpret_cast<uint8*>(LatestImage.Pixels.GetData());

				for (int32 Row = 0; Row < SurfaceHeight; ++Row)
				{
					FMemory::Memcpy(DestinationRow, SourceRow, DestinationPitch);
					SourceRow += SourcePitch;
					DestinationRow += DestinationPitch;
				}

				RHICmdList.UnmapStagingSurface(Slot->StagingTexture);
				Slot->CopyFence.SafeRelease();
				LatestImage.bHasImage = true;

				// We are done, give back the slot.
				Slot->SetState(EAGX_CameraSensorSlotState::Free);
			}
		});
}

void UAGX_CameraSensorComponent::JosefDebug() const
{
	const FCameraBarrier* CameraBarrier = GetNativeAsCamera();
	if (CameraBarrier == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Camera Sensor Component '%s' in '%s' JosefDebug: no native Camera."), *GetName(),
			*GetLabelSafe(GetOwner()));
		return;
	}

	const FCameraLatestImage& LatestImage = CameraBarrier->LatestImage;
	UE_LOG(
		LogAGX, Warning,
		TEXT("Camera Sensor Component '%s' in '%s' JosefDebug: LatestImage has %d pixels, "
			 "Resolution '%s', bHasImage %s."),
		*GetName(), *GetLabelSafe(GetOwner()), LatestImage.Pixels.Num(),
		*LatestImage.Resolution.ToString(), LatestImage.bHasImage ? TEXT("true") : TEXT("false"));
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
		SetupRenderPasses();

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

	if (FCameraBarrier* CameraBarrier = GetNativeAsCamera())
		CameraBarrier->LatestImage.Reset();

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
		SetupRenderPasses();
	}
}

void UAGX_CameraSensorComponent::TickComponent(
	float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	PollCapture();
}

void UAGX_CameraSensorComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);

	if (OwnedCaptureComponent2D != nullptr)
	{
		OwnedCaptureComponent2D->DestroyComponent();
		OwnedCaptureComponent2D = nullptr;
	}

	MaterialInstances.Empty();
	RenderTargets.Empty();
	SceneRenderTarget = nullptr;
	// TODO: consider CaptureHelper and in-flight requests here.

	if (FCameraBarrier* CameraBarrier = GetNativeAsCamera())
		CameraBarrier->LatestImage.Reset();
}

#if WITH_EDITOR
bool UAGX_CameraSensorComponent::CanEditChange(const FProperty* InProperty) const
{
	const bool SuperCanEditChange = Super::CanEditChange(InProperty);
	if (!SuperCanEditChange)
		return false;

	if (InProperty == nullptr)
		return SuperCanEditChange;

	if (InProperty->GetFName() == GET_MEMBER_NAME_CHECKED(ThisClass, Resolution) &&
		HasCaptureSourceOverride())
	{
		return false;
	}

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

void UAGX_CameraSensorComponent::SetupRenderPasses()
{
	MaterialInstances.Empty();

	if (GetCaptureSource() == nullptr)
		return;

	const FIntPoint ActiveResolution = GetActiveResolution();
	if (!IsResolutionValid(ActiveResolution))
		return;

	for (const TObjectPtr<UMaterialInterface>& Material : MaterialPasses)
	{
		if (Material == nullptr)
			continue;

		MaterialInstances.Add(UMaterialInstanceDynamic::Create(Material.Get(), this));
	}

	EnsureRenderTargets();
}

void UAGX_CameraSensorComponent::EnsureRenderTargets()
{
	USceneCaptureComponent2D* CaptureSource = GetCaptureSource();
	if (CaptureSource == nullptr)
		return;

	if (HasCaptureSourceOverride())
	{
		SceneRenderTarget = CaptureSource->TextureTarget;
		if (SceneRenderTarget == nullptr)
		{
			UE_LOG(
				LogAGX, Warning,
				TEXT("Camera Sensor Component '%s' in '%s' is using CaptureSourceOverride but "
					 "the referenced Scene Capture Component 2D '%s' does not have a render "
					 "target. Camera pipeline rendering will be skipped."),
				*GetName(), *GetLabelSafe(GetOwner()), *CaptureSource->GetName());
			return;
		}
	}

	const FIntPoint ActiveResolution = GetActiveResolution();
	if (!IsResolutionValid(ActiveResolution))
		return;

	if (!HasCaptureSourceOverride())
	{
		if (!IsRenderTargetUpToDate(SceneRenderTarget, ActiveResolution))
			SceneRenderTarget = CreateRenderTarget(ActiveResolution);

		CaptureSource->TextureTarget = SceneRenderTarget;
	}

	if (RenderTargets.Num() < MaterialInstances.Num())
		RenderTargets.SetNum(MaterialInstances.Num());

	for (int32 Index = 0; Index < MaterialInstances.Num(); ++Index)
	{
		if (!IsRenderTargetUpToDate(RenderTargets[Index], ActiveResolution))
			RenderTargets[Index] = CreateRenderTarget(ActiveResolution);
	}
}

FIntPoint UAGX_CameraSensorComponent::GetActiveResolution() const
{
	if (!HasCaptureSourceOverride())
		return Resolution;

	const USceneCaptureComponent2D* CaptureSource = GetCaptureSource();
	if (CaptureSource == nullptr || CaptureSource->TextureTarget == nullptr)
		return {0, 0};

	return {CaptureSource->TextureTarget->SizeX, CaptureSource->TextureTarget->SizeY};
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

	AGX_COMPONENT_DEFAULT_DISPATCHER(Resolution);
	AGX_COMPONENT_DEFAULT_DISPATCHER(MaterialInputTextureParameterName);
	PropertyDispatcher.Add(
		AGX_MEMBER_NAME(CaptureSourceOverride),
		[](ThisClass* This)
		{
			This->SetCaptureSourceOverride(
				This->CaptureSourceOverride.GetSceneCaptureComponent2D());
		});
	PropertyDispatcher.Add(
		AGX_MEMBER_NAME(MaterialPasses), [](ThisClass* This) { This->SetupRenderPasses(); });
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
