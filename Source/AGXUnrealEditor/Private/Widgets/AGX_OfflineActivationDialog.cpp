// Copyright 2025, Algoryx Simulation AB.

#include "Widgets/AGX_OfflineActivationDialog.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Environment.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SAGX_OfflineActivationDialog"

void SAGX_OfflineActivationDialog::Construct(const FArguments& InArgs)
{
	// clang-format off
	ChildSlot
	[
		SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f))
		.Padding(FMargin(5.0f, 5.0f))
		.Content()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(FMargin(5.0f, 5.0f))
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f))
				.BorderImage(FAGX_EditorUtilities::GetBrush("ToolPanel.GroupBorder"))
				.Padding(FMargin(5.0f, 5.0f))
				.Content()
				[
					CreateActivationRequestGui()
				]
			]
			+ SVerticalBox::Slot()
			.Padding(FMargin(5.0f, 5.0f))
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f))
				.BorderImage(FAGX_EditorUtilities::GetBrush("ToolPanel.GroupBorder"))
				.Padding(FMargin(5.0f, 5.0f))
				.Content()
				[
					CreateActivationResponseGui()
				]
			]
		]
	];
	// clang-format on
}

namespace AGX_OfflineActivationDialog_helpers
{
	FSlateFontInfo CreateFont(int Size)
	{
		FSlateFontInfo F = IPropertyTypeCustomizationUtils::GetRegularFont();
		F.Size = Size;
		return F;
	};
}

TSharedRef<SWidget> SAGX_OfflineActivationDialog::CreateActivationRequestGui()
{
	using namespace AGX_OfflineActivationDialog_helpers;
	// clang-format off
	static const FString InfoText =
		"Generate an offline activation request file. This file can\n"
		"then be used to generate an offline activation response\n"
		"on any other computer connected to the internet.\n\n"
		"Lastly, the offline activation response file can be used\n"
		"to generate a hardware bound service license file that\n"
		"will be automatically created and stored on this computer.\n\n"
		"More information regarding offline license activation can\n"
		"be found in the AGX Dynamics for Unreal User Manual.\n";

	return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(FMargin(50.0f, 10.0f, 10.f, 10.f))
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("GenerateOfflineRequestText", "Generate offline activation request"))
				.Font(CreateFont(12))
			]
			+ SVerticalBox::Slot()
			.Padding(FMargin(5.f, 5.0f))
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(InfoText))
				.Font(CreateFont(10))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(5.f, 5.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(FMargin(0.f, 0.f, 55.f, 0.f))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("LicenseIdText", "License id:"))
					.Font(CreateFont(10))
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SEditableTextBox)
					.Text(this, &SAGX_OfflineActivationDialog::GetLicenseIdText)
					.OnTextCommitted(this, &SAGX_OfflineActivationDialog::OnLicenseIdTextCommitted)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(5.f, 5.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(FMargin(0.f, 0.f, 21.f, 0.f))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ActivationCodeText", "Activation code:"))
					.Font(CreateFont(10))
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SEditableTextBox)
					.Text(this, &SAGX_OfflineActivationDialog::GetActivationCodeText)
					.OnTextCommitted(this, &SAGX_OfflineActivationDialog::OnActivationCodeCommitted)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(5.f, 5.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("SelectButtonText", "Generate..."))
					.ToolTipText(LOCTEXT("SelectButtonTooltip",
						"Select an output file that will contain the offline activation request."))
					.OnClicked(this, &SAGX_OfflineActivationDialog::OnGenerateActivationRequestButtonClicked)
				]
			];
	// clang-format on
}

TSharedRef<SWidget> SAGX_OfflineActivationDialog::CreateActivationResponseGui()
{
	using namespace AGX_OfflineActivationDialog_helpers;

	// clang-format off
	static const FString InfoText =
		"Use an offline activation response to generate a service license\n"
		"file. Once completed, a license file (agx.lfx) that is\n"
		"hardware bound to this computer will be created.\n";

	return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(FMargin(50.0f, 10.0f, 10.f, 10.f))
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ActivateOfflineResponseText", "Process offline activation response"))
				.Font(CreateFont(12))
			]
			+ SVerticalBox::Slot()
			.Padding(FMargin(5.f, 5.0f))
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::FromString(InfoText))
				.Font(CreateFont(10))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(5.f, 5.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(FMargin(0.f, 0.f, 33.f, 0.f))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ResponseFilePathText", "Response file:"))
					.Font(CreateFont(10))
				]
				+ SHorizontalBox::Slot()
				.Padding(FMargin(0.f, 0.f, 5.f, 0.f))
				.AutoWidth()
				[
					SNew(SEditableTextBox)
					.MinDesiredWidth(150.0f)
					.Text(this, &SAGX_OfflineActivationDialog::GetActivationResponsePathText)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("BrowseButtonText", "Browse..."))
					.ToolTipText(LOCTEXT("BrowseButtonTooltip",
						"Browse to an offline activation response file to be used for activating "
						"the service license."))
					.OnClicked(this, &SAGX_OfflineActivationDialog::OnBrowseResponseFileButtonClicked)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(5.0f, 5.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("GenerateLicenseButtonText", "  Generate license  "))
					.ToolTipText(LOCTEXT("GenerateLicenseButtonTooltip",
						"Generate a hardware bound service license file given the selected "
						"offline activation response file."))
					.OnClicked(this, &SAGX_OfflineActivationDialog::OnGenerateLicenseButtonClicked)
				]
			];
	// clang-format on
}

FText SAGX_OfflineActivationDialog::GetLicenseIdText() const
{
	return FText::FromString(LicenseId);
}

void SAGX_OfflineActivationDialog::OnLicenseIdTextCommitted(
	const FText& NewText, ETextCommit::Type InTextCommit)
{
	LicenseId = NewText.ToString();
}

FText SAGX_OfflineActivationDialog::GetActivationCodeText() const
{
	return FText::FromString(ActivationCode);
}

void SAGX_OfflineActivationDialog::OnActivationCodeCommitted(
	const FText& NewText, ETextCommit::Type InTextCommit)
{
	ActivationCode = NewText.ToString();
}

FReply SAGX_OfflineActivationDialog::OnGenerateActivationRequestButtonClicked()
{
	if (LicenseId.IsEmpty() || ActivationCode.IsEmpty())
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithError(
			"License Id or Activation code was empty. Please enter a License Id and Activation "
			"code.");
		return FReply::Handled();
	}

	if (!ContainsOnlyIntegers(LicenseId))
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithError(
			"License id may only contain integer values.");
		return FReply::Handled();
	}

	const FString Filename = FAGX_EditorUtilities::SelectNewFileDialog(
		"Select an activation request output file", "Text file|*.txt", "ActivationRequest.txt", "");

	if (Filename.IsEmpty())
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithError(
			"No output file was selected, could not generate offline activation request.");
		return FReply::Handled();
	}

	const int32 Id = FCString::Atoi(*LicenseId);
	const auto Output = FAGX_Environment::GetInstance().GenerateOfflineActivationRequest(
		Id, ActivationCode, Filename);
	if (!Output)
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithError(
			"Could not generate an offline activation request file. "
			"The Output Log may contain more information.");
		return FReply::Handled();
	}

	FAGX_NotificationUtilities::ShowDialogBoxWithInfo(
		"Offline activation request saved to: " + Output.GetValue());
	return FReply::Handled();
}

FReply SAGX_OfflineActivationDialog::OnBrowseResponseFileButtonClicked()
{
	ActivationResponsePath = FAGX_EditorUtilities::SelectExistingFileDialog(
		"Select offline activation response file", "");
	return FReply::Handled();
}

FText SAGX_OfflineActivationDialog::GetActivationResponsePathText() const
{
	return FText::FromString(ActivationResponsePath);
}

FReply SAGX_OfflineActivationDialog::OnGenerateLicenseButtonClicked()
{
	if (ActivationResponsePath.IsEmpty())
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithError(
			"Could not activate service license, no activation response file is selected.");
		return FReply::Handled();
	}

	const auto OutputFile =
		FAGX_Environment::GetInstance().ProcessOfflineActivationResponse(ActivationResponsePath);
	if (!OutputFile)
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithError(
			"Activating service license failed. The Output Log may contain more information.");
		return FReply::Handled();
	}

	FAGX_NotificationUtilities::ShowDialogBoxWithSuccess(
		"Activating service license was successful. The service license file is written to: " +
		OutputFile.GetValue());
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
