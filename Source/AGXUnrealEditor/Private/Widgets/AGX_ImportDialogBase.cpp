// Copyright 2024, Algoryx Simulation AB.

#include "Widgets/AGX_ImportDialogBase.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_ImportUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_SlateUtilities.h"

// Unreal Engine includes.
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SAGX_ImportDialogBase"

void SAGX_ImportDialogBase::SetFilePath(const FString& InFilePath)
{
	FilePath = InFilePath;
}

void SAGX_ImportDialogBase::SetIgnoreDisabledTrimeshes(bool bInIgnoreDisabledTrimesh)
{
	bIgnoreDisabledTrimesh = bInIgnoreDisabledTrimesh;
}

TSharedRef<SWidget> SAGX_ImportDialogBase::CreateBrowseFileGui()
{
	// clang-format off
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.Padding(FMargin(10.0f, 10.0f, 10.f, 10.f))
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("BrowseFileText", "Select file"))
			.Font(FAGX_SlateUtilities::CreateFont(12))
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
				.Text(LOCTEXT("FilePathText", "File:"))
				.Font(FAGX_SlateUtilities::CreateFont(10))
			]
			+ SHorizontalBox::Slot()
			.Padding(FMargin(0.f, 0.f, 5.f, 0.f))
			.AutoWidth()
			[
				SNew(SEditableTextBox)
				.MinDesiredWidth(150.0f)
				.Text(this, &SAGX_ImportDialogBase::GetFilePathText)
				.OnTextCommitted(this, &SAGX_ImportDialogBase::OnFilePathTextCommitted)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("BrowseButtonText", "Browse..."))
				.ToolTipText(LOCTEXT("BrowseButtonTooltip",
					"Browse to a file."))
				.OnClicked(this, &SAGX_ImportDialogBase::OnBrowseFileButtonClicked)
			]
		];
	// clang-format on
}

TSharedRef<SBorder> SAGX_ImportDialogBase::CreateAGXFileGui()
{
	// Currently no AGX File specific GUI elements to show. This can be expanded in the future if
	// needed.
	return MakeShared<SBorder>();
}

TSharedRef<SBorder> SAGX_ImportDialogBase::CreatePLXFileGui()
{
	if (ImportType != EAGX_ImportType::Plx)
	{
		return MakeShared<SBorder>();
	}

	// clang-format off
	return SNew(SBorder)
				.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f))
				.BorderImage(FAGX_EditorUtilities::GetBrush("ToolPanel.GroupBorder"))
				.Padding(FMargin(5.0f, 15.0f))
				.Content()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(FMargin(0.f, 0.f, 33.f, 0.f))
						[
							SNew(STextBlock)
							.ColorAndOpacity(FLinearColor(1.0f, 0.45f, 0, 1.0f))
							.Text(LOCTEXT("PLXExperimentalTesxt", "Note: OpenPLX support is currently Experimental, "
								"meaning the supported features \nare limited and backwards compatibility in "
								"future releases is not guaranteed."))
							.Font(FAGX_SlateUtilities::CreateFont(10))
						]
					]
				];
	// clang-format on
}

TSharedRef<SWidget> SAGX_ImportDialogBase::CreateIgnoreDisabledTrimeshGui()
{
	// clang-format off
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.Padding(FMargin(0.f, 0.f, 5.f, 0.f))
			.AutoWidth()
			[
				SNew(SCheckBox)
					.ToolTipText(LOCTEXT("IgnoreDisabledTrimeshCheckBoxTooltip",
						"Any Trimesh that has collision disabled will be ignored. "
						"Only its visual representation will be imported."))
					.OnCheckStateChanged(this, &SAGX_ImportDialogBase::OnIgnoreDisabledTrimeshCheckboxClicked)
					.IsChecked(bIgnoreDisabledTrimesh)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(0.f, 0.f, 33.f, 0.f))
			[
				SNew(STextBlock)
				.Text(LOCTEXT("IgnoreDisabledTrimeshText", "Ignore disabled Trimeshes (recommended for large models)"))
				.Font(FAGX_SlateUtilities::CreateFont(10))
			]
		];
	// clang-format on
}

FReply SAGX_ImportDialogBase::OnBrowseFileButtonClicked()
{
	const FString CurrentSelectedDir = FPaths::GetPath(FilePath);
	const FString StartDir = FPaths::DirectoryExists(CurrentSelectedDir) ? CurrentSelectedDir : "";
	FilePath = FAGX_EditorUtilities::SelectExistingFileDialog("file", FileTypes, StartDir);
	ImportType = FAGX_ImportUtilities::GetFrom(FilePath);

	RefreshGui();

	if (ImportType == EAGX_ImportType::Invalid && !FilePath.IsEmpty())
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithError(
			"Unable to detect file type for selected type.");
		FilePath = "";
		return FReply::Handled();
	}

	return FReply::Handled();
}

FText SAGX_ImportDialogBase::GetFilePathText() const
{
	return FText::FromString(FilePath);
}

void SAGX_ImportDialogBase::OnIgnoreDisabledTrimeshCheckboxClicked(ECheckBoxState NewCheckedState)
{
	bIgnoreDisabledTrimesh = NewCheckedState == ECheckBoxState::Checked;
}

void SAGX_ImportDialogBase::RefreshGui()
{
	Construct(FArguments());
}

void SAGX_ImportDialogBase::OnFilePathTextCommitted(
	const FText& InNewText, ETextCommit::Type InCommitType)
{
	FilePath = InNewText.ToString();
	ImportType = FAGX_ImportUtilities::GetFrom(FilePath);
	RefreshGui();
}

#undef LOCTEXT_NAMESPACE
