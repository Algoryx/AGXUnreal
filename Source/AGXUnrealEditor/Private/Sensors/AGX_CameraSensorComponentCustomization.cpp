// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_CameraSensorComponentCustomization.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "Sensors/AGX_CameraSensorBase.h"
#include "Sensors/AGX_CameraSensor8BitComponent.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "Input/Reply.h"
#include "Misc/Paths.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"

#define LOCTEXT_NAMESPACE "FAGX_CameraSensorComponentCustomization"

TSharedRef<IDetailCustomization> FAGX_CameraSensorComponentCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_CameraSensorComponentCustomization);
}

void FAGX_CameraSensorComponentCustomization::CustomizeDetails(
	IDetailLayoutBuilder& InDetailBuilder)
{
	DetailBuilder = &InDetailBuilder;

	UAGX_CameraSensorBase* CameraSensorComponent =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_CameraSensorBase>(
			InDetailBuilder);
	if (CameraSensorComponent == nullptr)
	{
		return;
	}

	IDetailCategoryBuilder& CategoryBuilder =
		InDetailBuilder.EditCategory("AGX Camera", FText::GetEmpty(), ECategoryPriority::Important);

	CategoryBuilder.AddProperty(
		InDetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAGX_CameraSensorBase, FOV)));
	CategoryBuilder.AddProperty(InDetailBuilder.GetProperty(
		GET_MEMBER_NAME_CHECKED(UAGX_CameraSensorBase, Resolution)));

	// clang-format off
	CategoryBuilder.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("GenerateRuntimeAssetsButtonText", "Generate Runtime Assets"))
			.ToolTipText(LOCTEXT(
				"GenerateRuntimeAssetsTooltip",
				"Generates and sets needed runtime Assets for this Camera Component given the FOV and Resolution.\n"
				"Notice that this has to be done again if any of these settings are changed."
				"If needed runtime Assets are already set, the existing Assets will be updated."))
			.OnClicked(this, &FAGX_CameraSensorComponentCustomization::OnGenerateRuntimeAssetsButtonClicked)
		]
	];
	// clang-format on

	InDetailBuilder.HideCategory(FName("Sockets"));
}

namespace AGX_CameraSensorComponentCustomization_helpers
{
	UAGX_CameraSensorBase* GetCameraSensorComponent(IDetailLayoutBuilder* DetailBuilder)
	{
		if (DetailBuilder == nullptr)
		{
			FAGX_NotificationUtilities::ShowNotification(
				"Unable to get the DetailBuilder from OnGenerateRuntimeAssetsButtonClicked. No "
				"action will be performed.",
				SNotificationItem::ECompletionState::CS_Fail);
			return nullptr;
		}

		UAGX_CameraSensorBase* CameraComponent =
			FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_CameraSensorBase>(
				*DetailBuilder);
		if (CameraComponent == nullptr)
		{
			FAGX_NotificationUtilities::ShowNotification(
				"Unable to get the Camera Sensor Component from "
				"OnGenerateRuntimeAssetsButtonClicked. "
				"No action will be performed.",
				SNotificationItem::ECompletionState::CS_Fail);
			return nullptr;
		}

		if (CameraComponent->GetWorld() != nullptr && CameraComponent->GetWorld()->IsGameWorld())
		{
			FAGX_NotificationUtilities::ShowNotification(
				"This action is not possible during Play.",
				SNotificationItem::ECompletionState::CS_Fail);
			return nullptr;
		}

		return CameraComponent;
	}

	template <typename T>
	T* CreateAsset(const FString& AssetNameSuggestion, const FString& DialogTitle)
	{
		const FString AssetPath = FAGX_EditorUtilities::SelectNewAssetDialog(
			T::StaticClass(), "", AssetNameSuggestion, DialogTitle);

		if (AssetPath.IsEmpty())
			return nullptr;

		const FString AssetName = FPaths::GetBaseFilename(AssetPath);

		UPackage* Package = CreatePackage(*AssetPath);
		T* Asset = NewObject<T>(Package, FName(*AssetName), RF_Public | RF_Standalone);
		if (Asset == nullptr)
		{
			FAGX_NotificationUtilities::ShowNotification(
				FString::Printf(TEXT("Unable to create asset given Asset Path: '%s'"), *AssetPath),
				SNotificationItem::ECompletionState::CS_Fail);
			return nullptr;
		}

		return Asset;
	}

	bool IsSettingsValid(const UAGX_CameraSensorBase& CameraComponent)
	{
		auto IsNearlyZero = [](const FVector2D& V)
		{ return FMath::IsNearlyZero(V.X) || FMath::IsNearlyZero(V.Y); };

		if (IsNearlyZero(CameraComponent.Resolution) || FMath::IsNearlyZero(CameraComponent.FOV))
		{
			FAGX_NotificationUtilities::ShowNotification(
				FString::Printf(
					TEXT("Camera Component '%s' has a FOV or Resolution too close to zero."),
					*CameraComponent.GetName()),
				SNotificationItem::ECompletionState::CS_Fail);
			return false;
		}

		return true;
	}
}

FReply FAGX_CameraSensorComponentCustomization::OnGenerateRuntimeAssetsButtonClicked()
{
	using namespace AGX_CameraSensorComponentCustomization_helpers;
	AGX_CHECK(DetailBuilder);

	UAGX_CameraSensorBase* CameraComponent = GetCameraSensorComponent(DetailBuilder);
	if (CameraComponent == nullptr)
		return FReply::Handled(); // Logging done in GetCameraSensorComponent.

	if (!IsSettingsValid(*CameraComponent))
		return FReply::Handled(); // Logging done in IsSettingsValid.

	// Ask the user to specify file name and location for the RenderTarget2D to be created.
	FString ComponentName = CameraComponent->GetName();
	ComponentName.RemoveFromEnd(
		UActorComponent::ComponentTemplateNameSuffix); // In case this is a Component Template.
	const FString DefaultAssetName = FString::Printf(TEXT("RT_Camera_%s"), *ComponentName);

	UTextureRenderTarget2D* RenderTargetAsset = CameraComponent->RenderTarget;
	if (RenderTargetAsset == nullptr)
	{
		// No Asset is assigned, create a new one.
		RenderTargetAsset = CreateAsset<UTextureRenderTarget2D>(
			DefaultAssetName, FString("Save Camera Render Target As"));
		if (RenderTargetAsset == nullptr)
			return FReply::Handled(); // Logging done in CreateAsset.
	}

	// Setup Render Target according to the Camera Sensor Component settings.
	RenderTargetAsset->ResizeTarget(CameraComponent->Resolution.X, CameraComponent->Resolution.Y);

	if (Cast<UAGX_CameraSensor8BitComponent>(CameraComponent) != nullptr)
		RenderTargetAsset->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
	else
		RenderTargetAsset->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA16f;

	if (!FAGX_ObjectUtilities::SaveAsset(*RenderTargetAsset))
	{
		FAGX_NotificationUtilities::ShowNotification(
			FString::Printf(TEXT("Unable to save asset: '%s'"), *RenderTargetAsset->GetName()),
			SNotificationItem::ECompletionState::CS_Fail);
		return FReply::Handled();
	}

	// Finally, assign the created Asset.
	for (auto Instance : FAGX_ObjectUtilities::GetArchetypeInstances(*CameraComponent))
	{
		if (Instance->RenderTarget == CameraComponent->RenderTarget)
		{
			Instance->RenderTarget = RenderTargetAsset;
		}
	}

	CameraComponent->RenderTarget = RenderTargetAsset;

	FAGX_NotificationUtilities::ShowDialogBoxWithLogLog(
		FString::Printf(TEXT("Successfully saved: '%s'"), *RenderTargetAsset->GetName()));

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
