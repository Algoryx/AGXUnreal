// Copyright 2025, Algoryx Simulation AB.

#include "Widgets/AGX_GenerateRuntimeActivationDialog.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Environment.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SAGX_GenerateRuntimeActivationDialog"

void SAGX_GenerateRuntimeActivationDialog::Construct(const FArguments& InArgs)
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
					CreateUserInputGui()
				]
			]
		]
	];
	// clang-format on
}

TSharedRef<SWidget> SAGX_GenerateRuntimeActivationDialog::CreateUserInputGui()
{
	auto CreateFont = [](int Size)
	{
		FSlateFontInfo F = IPropertyTypeCustomizationUtils::GetRegularFont();
		F.Size = Size;
		return F;
	};

	static const FString InfoText =
		"Generate an AGX Dynamics for Unreal runtime activation containing encrypted License id \n"
		"and Activation code bound to a built application. The generated runtime activation file \n"
		"(agx.rtlfx) will be replaced automatically by a hardware locked agx.lfx file once the \n"
		"application is started, if a successful activation could be performed.\n\n"
		" Internet access is required during the activation on the target hardware.\n\n";

	// clang-format off
	return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(FMargin(50.0f, 10.0f, 10.f, 10.f))
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("GenerateRuntimeActivationText", "Generate runtime activation"))
				.Font(CreateFont(16))
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
					.Text(this, &SAGX_GenerateRuntimeActivationDialog::GetLicenseIdText)
					.OnTextCommitted(this, &SAGX_GenerateRuntimeActivationDialog::OnLicenseIdTextCommitted)
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
					.Text(this, &SAGX_GenerateRuntimeActivationDialog::GetActivationCodeText)
					.OnTextCommitted(this, &SAGX_GenerateRuntimeActivationDialog::OnActivationCodeCommitted)
				]
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
					.Text(LOCTEXT("ReferenceFilePathText", "Reference file:"))
					.Font(CreateFont(10))
				]
				+ SHorizontalBox::Slot()
				.Padding(FMargin(0.f, 0.f, 5.f, 0.f))
				.AutoWidth()
				[
					SNew(SEditableTextBox)
					.MinDesiredWidth(150.0f)
					.Text(this, &SAGX_GenerateRuntimeActivationDialog::GetReferenceFilePathText)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("BrowseButtonText", "Browse..."))
					.ToolTipText(LOCTEXT("BrowseButtonTooltip",
						"Browse a unique reference file within the built application that a runtime activation "
						"will be generated for."))
					.OnClicked(this, &SAGX_GenerateRuntimeActivationDialog::OnBrowseReferenceFileButtonClicked)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(5.f, 5.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(FMargin(0.f, 0.f, 13.f, 0.f))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("LicenseDirPathText", "License directory:"))
					.ToolTipText(LOCTEXT("LicenseDirToolTip",
						"Browse to the AGX Dynamics for Unreal license directory within the built application."))
					.Font(CreateFont(10))
				]
				+ SHorizontalBox::Slot()
				.Padding(FMargin(0.f, 0.f, 5.f, 0.f))
				.AutoWidth()
				[
					SNew(SEditableTextBox)
					.MinDesiredWidth(150.0f)
					.Text(this, &SAGX_GenerateRuntimeActivationDialog::GetLicenseDirPathText)
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("BrowseButtonText2", "Browse..."))
					.ToolTipText(LOCTEXT("BrowseButtonTooltip2",
						"Browse to the AGX Dynamics for Unreal license directory within the built application."))
					.OnClicked(this, &SAGX_GenerateRuntimeActivationDialog::OnBrowseLicenseDirButtonClicked)
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
					.Text(LOCTEXT("GenerateButtonText", "  Generate  "))
					.ToolTipText(LOCTEXT("GenerateButtonTooltip",
						"Generate an AGX Dynamics runtime activation for the specified application "
						"given License id and Activation code."))
					.OnClicked(this, &SAGX_GenerateRuntimeActivationDialog::OnGenerateButtonClicked)
				]
			];
	// clang-format on
}

FText SAGX_GenerateRuntimeActivationDialog::GetLicenseIdText() const
{
	return FText::FromString(LicenseId);
}

void SAGX_GenerateRuntimeActivationDialog::OnLicenseIdTextCommitted(
	const FText& NewText, ETextCommit::Type InTextCommit)
{
	LicenseId = NewText.ToString();
}

FText SAGX_GenerateRuntimeActivationDialog::GetActivationCodeText() const
{
	return FText::FromString(ActivationCode);
}

void SAGX_GenerateRuntimeActivationDialog::OnActivationCodeCommitted(
	const FText& NewText, ETextCommit::Type InTextCommit)
{
	ActivationCode = NewText.ToString();
}

FReply SAGX_GenerateRuntimeActivationDialog::OnGenerateButtonClicked()
{
	if (LicenseId.IsEmpty() || ActivationCode.IsEmpty())
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithError(
			"License Id or Activation code was empty. Please enter a License Id and Activation "
			"code.");
		return FReply::Handled();
	}

	if (ReferenceFilePath.IsEmpty() || LicenseDirPath.IsEmpty())
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithError(
			"Reference file or License directory was empty. Please select a Reference file and "
			"License directory.");
		return FReply::Handled();
	}

	if (!ContainsOnlyIntegers(LicenseId))
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithError(
			"License id may only contain integer values.");
		return FReply::Handled();
	}

	const int32 Id = FCString::Atoi(*LicenseId);
	TOptional<FString> OutputFile = FAGX_Environment::GetInstance().GenerateRuntimeActivation(
		Id, ActivationCode, ReferenceFilePath, LicenseDirPath);

	if (!OutputFile.IsSet())
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithError(
			"License activation was unsuccessful. The Output Log may contain more information.");
		return FReply::Handled();
	}

	FAGX_NotificationUtilities::ShowDialogBoxWithSuccess(
		"Runtime activation generation was successful. Resulting encrypted file written to: " +
		OutputFile.GetValue());

	return FReply::Handled();
}

FText SAGX_GenerateRuntimeActivationDialog::GetReferenceFilePathText() const
{
	return FText::FromString(ReferenceFilePath);
}

FReply SAGX_GenerateRuntimeActivationDialog::OnBrowseReferenceFileButtonClicked()
{
	ReferenceFilePath = FAGX_EditorUtilities::SelectExistingFileDialog(
		"Select reference file in built application", "");
	return FReply::Handled();
}

FText SAGX_GenerateRuntimeActivationDialog::GetLicenseDirPathText() const
{
	return FText::FromString(LicenseDirPath);
}

FReply SAGX_GenerateRuntimeActivationDialog::OnBrowseLicenseDirButtonClicked()
{
	const FString StartDir = ReferenceFilePath.IsEmpty() ? "" : FPaths::GetPath(ReferenceFilePath);
	LicenseDirPath = FAGX_EditorUtilities::SelectExistingDirectoryDialog(
		"Select AGX Dynamics for Unreal license directory within the built application", StartDir);
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
