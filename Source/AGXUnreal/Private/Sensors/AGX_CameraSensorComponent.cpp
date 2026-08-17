// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_CameraSensorComponent.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "Sensors/AGX_CameraBackend.h"
#include "Sensors/AGX_SensorEnvironmentSubsystem.h"
#include "Sensors/CameraBarrier.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UAGX_CameraSensorComponent::UAGX_CameraSensorComponent()
{
	NativeBarrier.Reset(new FCameraBarrier());
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
}

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

	CaptureComponent2D = NewObject<USceneCaptureComponent2D>(Owner, NAME_None);
	CaptureComponent2D->SetupAttachment(this);
	CaptureComponent2D->SetCanEverAffectNavigation(false);
	CaptureComponent2D->CreationMethod = CreationMethod;
	CaptureComponent2D->RegisterComponentWithWorld(World);
}
