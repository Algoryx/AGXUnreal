// Copyright 2025, Algoryx Simulation AB.

#include "Sensors/AGX_CameraSensorBase.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "AGX_PropertyChangedDispatcher.h"
#include "AGX_Simulation.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"

UAGX_CameraSensorBase::UAGX_CameraSensorBase()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAGX_CameraSensorBase::SetFOV(float InFOV)
{
	if (!IsFovValid(InFOV))
		return;

	if (CaptureComponent2D != nullptr)
		CaptureComponent2D->FOVAngle = InFOV;

	FOV = InFOV;
}

void UAGX_CameraSensorBase::SetResolution(FIntPoint InResolution)
{
	if (!IsResolutionValid(InResolution))
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("Camera Sensor '%s' in Actor '%s', invalid Resolution passed to SetResolution: "
				 "%s. Doing nothing."),
			*GetName(), *GetLabelSafe(GetOwner()), *InResolution.ToString());
		return;
	}

	Resolution = InResolution;
	if (RenderTarget != nullptr)
		RenderTarget->ResizeTarget(InResolution.X, InResolution.Y);
}

USceneCaptureComponent2D* UAGX_CameraSensorBase::GetSceneCaptureComponent2D() const
{
	return CaptureComponent2D;
}

bool UAGX_CameraSensorBase::IsFovValid(float FOV)
{
	return FOV > KINDA_SMALL_NUMBER && FOV <= 170.f;
}

bool UAGX_CameraSensorBase::IsResolutionValid(const FIntPoint& Resolution)
{
	return Resolution.X >= 1 && Resolution.Y >= 1;
}

void UAGX_CameraSensorBase::BeginPlay()
{
	Super::BeginPlay();

	if (GIsReconstructingBlueprintInstances)
		return;

	Init();
}

void UAGX_CameraSensorBase::PostApplyToComponent()
{
	Super::PostApplyToComponent();

	if (GIsReconstructingBlueprintInstances && GetWorld() && GetWorld()->IsGameWorld())
	{
		// We are in a Blueprint reconstruction during Play and we need to call Init manually to
		// ensure we are in a valid state. Otherwise CaptureComponent2D will always be nullptr since
		// it is not copied over to this new Component.
		// The reason we do this in PostApplyToComponent is because that's what was found to be
		// called after the new Component has been created and given all of it's properties from the
		// previous Component. There might be a better place to do this, but it is currently not
		// know what that place would be.
		Init();
	}
}

#if WITH_EDITOR
bool UAGX_CameraSensorBase::CanEditChange(const FProperty* InProperty) const
{
	const bool SuperCanEditChange = Super::CanEditChange(InProperty);
	if (!SuperCanEditChange)
		return false;

	if (InProperty == nullptr)
	{
		return SuperCanEditChange;
	}

	const bool bIsPlaying = GetWorld() && GetWorld()->IsGameWorld();
	if (bIsPlaying)
	{
		// List of names of properties that does not support editing after initialization.
		static const TArray<FName> PropertiesNotEditableDuringPlay = {
			GET_MEMBER_NAME_CHECKED(ThisClass, Resolution),
			GET_MEMBER_NAME_CHECKED(ThisClass, RenderTarget)};

		if (PropertiesNotEditableDuringPlay.Contains(InProperty->GetFName()))
		{
			return false;
		}
	}
	return SuperCanEditChange;
}

void UAGX_CameraSensorBase::PostEditChangeChainProperty(FPropertyChangedChainEvent& Event)
{
	FAGX_PropertyChangedDispatcher<ThisClass>::Get().Trigger(Event);

	// If we are part of a Blueprint then this will trigger a RerunConstructionScript on the owning
	// Actor. That means that this object will be removed from the Actor and destroyed. We want to
	// apply all our changes before that so that they are carried over to the copy.
	Super::PostEditChangeChainProperty(Event);
}

void UAGX_CameraSensorBase::PostInitProperties()
{
	Super::PostInitProperties();

	FAGX_PropertyChangedDispatcher<ThisClass>& PropertyDispatcher =
		FAGX_PropertyChangedDispatcher<ThisClass>::Get();
	if (PropertyDispatcher.IsInitialized())
	{
		return;
	}

	PropertyDispatcher.Add(
		GET_MEMBER_NAME_CHECKED(UAGX_CameraSensorBase, FOV),
		[](ThisClass* This) { This->SetFOV(This->FOV); });
}

#endif

void UAGX_CameraSensorBase::Init()
{
	AGX_CHECK(CaptureComponent2D == nullptr);

	CaptureComponent2D =
		NewObject<USceneCaptureComponent2D>(this, FName(TEXT("SceneCaptureComponent2D")));
	CaptureComponent2D->RegisterComponent();
	CaptureComponent2D->AttachToComponent(
		this, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	InitCaptureComponent();
	bIsValid = CheckValid();
}

void UAGX_CameraSensorBase::InitCaptureComponent()
{
	if (CaptureComponent2D == nullptr)
		return;

	CaptureComponent2D->TextureTarget = RenderTarget;
	CaptureComponent2D->FOVAngle = FOV;
	CaptureComponent2D->CaptureSource = ESceneCaptureSource::SCS_FinalToneCurveHDR;
}

bool UAGX_CameraSensorBase::CheckValid() const
{
	if (!IsFovValid(FOV))
	{
		const FString Msg = FString::Printf(
			TEXT(
				"Camera Sensor '%s' in Actor '%s' has an invalid FOV: %f. Please set a valid FOV."),
			*GetName(), *GetLabelSafe(GetOwner()), FOV);
		FAGX_NotificationUtilities::ShowNotification(Msg, SNotificationItem::CS_Fail);
		return false;
	}

	if (!IsResolutionValid(Resolution))
	{
		const FString Msg = FString::Printf(
			TEXT("Camera Sensor '%s' in Actor '%s' has an invalid Resolution: [%s]. Please set a "
				 "valid Resolution."),
			*GetName(), *GetLabelSafe(GetOwner()), *Resolution.ToString());
		FAGX_NotificationUtilities::ShowNotification(Msg, SNotificationItem::CS_Fail);
		return false;
	}

	if (RenderTarget == nullptr)
	{
		const FString Msg = FString::Printf(
			TEXT("Camera Sensor '%s' in Actor '%s' does not have a RenderTarget assigned to it. "
				 "Use the 'Generate Runtime Assets' button in the Details Panel to generate a "
				 "valid RenderTarget."),
			*GetName(), *GetLabelSafe(GetOwner()));
		FAGX_NotificationUtilities::ShowNotification(Msg, SNotificationItem::CS_Fail);
		return false;
	}
	else if (RenderTarget->SizeX != Resolution.X || RenderTarget->SizeY != Resolution.Y)
	{
		const FString Msg = FString::Printf(
			TEXT("Camera Sensor '%s' in Actor '%s' expected to have a RenderTarget with the "
				 "resolution [%s] but it was [%d %d]. "
				 "Use the 'Generate Runtime Assets' button in the Details Panel to generate a "
				 "valid RenderTarget."),
			*GetName(), *GetLabelSafe(GetOwner()), *Resolution.ToString(), RenderTarget->SizeX,
			RenderTarget->SizeY);
		FAGX_NotificationUtilities::ShowNotification(Msg, SNotificationItem::CS_Fail);
		return false;
	}

	if (CaptureComponent2D == nullptr)
	{
		const FString Msg = FString::Printf(
			TEXT("Camera Sensor '%s' in Actor '%s' does not have a CaptureComponent2D subobject."),
			*GetName(), *GetLabelSafe(GetOwner()));
		FAGX_NotificationUtilities::ShowNotification(Msg, SNotificationItem::CS_Fail);
		return false;
	}

	return true;
}
