// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_CameraSensorComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "Sensors/AGX_CameraBackend.h"
#include "Sensors/AGX_SensorEnvironmentSubsystem.h"
#include "Sensors/CameraBarrier.h"
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

UAGX_CameraSensorComponent::UAGX_CameraSensorComponent()
{
	NativeBarrier.Reset(new FCameraBarrier());
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
		MaterialPasses.RemoveAll(
			[Material](const TObjectPtr<UMaterialInterface>& ExistingMaterial)
			{
				return ExistingMaterial.Get() == Material;
			});
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

USceneCaptureComponent2D* UAGX_CameraSensorComponent::GetSceneCaptureComponent2D() const
{
	return CaptureComponent2D;
}

bool UAGX_CameraSensorComponent::IsCameraSensorValid() const
{
	return CaptureComponent2D != nullptr && HasNative();
}

UTextureRenderTarget2D* UAGX_CameraSensorComponent::RenderCameraPipeline()
{
	if (CaptureComponent2D == nullptr)
		return nullptr;

	EnsureRenderTargets();
	if (SceneRenderTarget == nullptr)
		return nullptr;

	CaptureComponent2D->CaptureScene();

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
				FVector2D(Resolution.X, Resolution.Y), FVector2D(0.0f, 0.0f),
				FVector2D(1.0f, 1.0f));
			Item.BlendMode = SE_BLEND_Opaque;
			Canvas->DrawItem(Item);
		}

		UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(this, DrawContext);

		FinalRenderTarget = OutputRenderTarget;
		InputTexture = FinalRenderTarget;
	}

	return FinalRenderTarget;
}

FSensorBarrier* UAGX_CameraSensorComponent::CreateNativeImpl()
{
	Super::CreateNativeImpl();

	AGX_CHECK(!HasNative());
	if (HasNative())
		return GetNativeAsCamera();

	auto CameraBackend = UAGX_CameraBackend::GetFrom(this);
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

	FCameraBarrier* CameraBarrier = GetNativeAsCamera();
	if (CameraBarrier == nullptr)
		return nullptr;

	CameraBarrier->AllocateNative(GetComponentTransform(), *CameraBackendBarrier);
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
		SetupSceneCapture();
		SetupRenderPasses();
	}
}

void UAGX_CameraSensorComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);

	if (CaptureComponent2D != nullptr)
	{
		CaptureComponent2D->DestroyComponent();
		CaptureComponent2D = nullptr;
	}

	MaterialInstances.Empty();
	RenderTargets.Empty();
	SceneRenderTarget = nullptr;
}

#if WITH_EDITOR
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

FCameraBarrier* UAGX_CameraSensorComponent::GetNativeAsCamera()
{
	if (NativeBarrier == nullptr)
		return nullptr;

	return static_cast<FCameraBarrier*>(NativeBarrier.Get());
}

const FCameraBarrier* UAGX_CameraSensorComponent::GetNativeAsCamera() const
{
	if (NativeBarrier == nullptr)
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
	if (CaptureComponent2D != nullptr)
		return;

	AActor* Owner = GetOwner();
	if (Owner == nullptr)
		return;

	UWorld* World = GetWorld();
	if (World == nullptr)
		return;

	CaptureComponent2D =
		NewObject<USceneCaptureComponent2D>(this, FName(TEXT("SceneCaptureComponent2D")));
	CaptureComponent2D->CreationMethod = EComponentCreationMethod::Native;
	CaptureComponent2D->RegisterComponent();
	CaptureComponent2D->AttachToComponent(
		this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	CaptureComponent2D->SetCanEverAffectNavigation(false);
	CaptureComponent2D->bCaptureEveryFrame = false;
	CaptureComponent2D->bCaptureOnMovement = false;
	CaptureComponent2D->bAlwaysPersistRenderingState = true;
}

void UAGX_CameraSensorComponent::SetupRenderPasses()
{
	MaterialInstances.Empty();

	if (CaptureComponent2D == nullptr)
		return;

	if (!IsResolutionValid(Resolution))
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
	if (CaptureComponent2D == nullptr)
		return;

	if (!IsResolutionValid(Resolution))
		return;

	if (!IsRenderTargetUpToDate(SceneRenderTarget))
		SceneRenderTarget = CreateRenderTarget();

	CaptureComponent2D->TextureTarget = SceneRenderTarget;

	if (RenderTargets.Num() < MaterialInstances.Num())
		RenderTargets.SetNum(MaterialInstances.Num());

	for (int32 Index = 0; Index < MaterialInstances.Num(); ++Index)
	{
		if (!IsRenderTargetUpToDate(RenderTargets[Index]))
			RenderTargets[Index] = CreateRenderTarget();
	}
}

UTextureRenderTarget2D* UAGX_CameraSensorComponent::CreateRenderTarget()
{
	UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>(this);
	RenderTarget->bGPUSharedFlag = true;
	RenderTarget->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
	RenderTarget->TargetGamma = 2.2f;
	RenderTarget->InitAutoFormat(Resolution.X, Resolution.Y);
	return RenderTarget;
}

bool UAGX_CameraSensorComponent::IsRenderTargetUpToDate(
	const UTextureRenderTarget2D* RenderTarget) const
{
	return RenderTarget != nullptr && RenderTarget->SizeX == Resolution.X &&
		   RenderTarget->SizeY == Resolution.Y;
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
		AGX_MEMBER_NAME(MaterialPasses), [](ThisClass* This) { This->SetupRenderPasses(); });
}
#endif
