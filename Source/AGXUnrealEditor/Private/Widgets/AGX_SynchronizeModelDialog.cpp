// Copyright 2024, Algoryx Simulation AB.

#include "Widgets/AGX_SynchronizeModelDialog.h"

// AGX Dynamics for Unreal includes.
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_ImportUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_SlateUtilities.h"

// Unreal Engine includes.
#include "Framework/Application/SlateApplication.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "SAGX_SynchronizeModelDialog"

void SAGX_SynchronizeModelDialog::Construct(const FArguments& InArgs)
{
	FileTypes = ".agx;*.openplx";
	ImportType = EAGX_ImportType::Agx;

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
					CreateBrowseFileGui()
				]
			]
			+ SVerticalBox::Slot()
			.Padding(FMargin(5.0f, 0.0f))
			.AutoHeight()
			[
				CreateAGXFileGui()
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(5.0f, 5.0f))
			[
				CreateSettingsGui()
			]
			+ SVerticalBox::Slot()
			.Padding(FMargin(5.0f, 5.0f))
			.AutoHeight()
			[
				CreateSynchronizeButtonGui()
			]
		]
	];
	// clang-format on
}

TOptional<FAGX_SynchronizeModelSettings> SAGX_SynchronizeModelDialog::ToSynchronizeModelSettings()
{
	if (!bUserHasPressedImportOrSynchronize)
	{
		// The Window containing this Widget was closed, the user never pressed Synchronize.
		return {};
	}

	if (FilePath.IsEmpty())
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
			"A file must be selected before synchronizing the model.");
		return {};
	}

	FAGX_SynchronizeModelSettings Settings;
	Settings.FilePath = FilePath;
	Settings.bIgnoreDisabledTrimeshes = bIgnoreDisabledTrimesh;
	Settings.bForceOverwriteProperties = bForceOverwriteProperties;
	Settings.bForceReassignRenderMaterials = bForceReassignRenderMaterials;
	Settings.ImportType = FAGX_ImportUtilities::GetFrom(FilePath);
	return Settings;
}

TSharedRef<SBorder> SAGX_SynchronizeModelDialog::CreateSettingsGui()
{
	if (ImportType == EAGX_ImportType::Invalid)
	{
		return MakeShared<SBorder>();
	}

	// clang-format off
	return SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f))
		.BorderImage(FAGX_EditorUtilities::GetBrush("ToolPanel.GroupBorder"))
		.Padding(FMargin(5.0f, 5.0f))
		.Content()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(FMargin(10.0f, 10.0f, 10.f, 10.f))
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SettingsText", "Settings"))
				.Font(FAGX_SlateUtilities::CreateFont(12))
			]
			+ SVerticalBox::Slot()
			.Padding(FMargin(50.0f, 10.0f, 0.f, 10.f))
			.AutoHeight()
			[
				CreateIgnoreDisabledTrimeshGui()
			]
			+ SVerticalBox::Slot()
			.Padding(FMargin(50.0f, 0.0f, 0.f, 10.f))
			.AutoHeight()
			[
				CreateForceOverwritePropertiesGui()
			]
			+ SVerticalBox::Slot()
			.Padding(FMargin(50.0f, 0.0f, 10.f, 10.f))
			.AutoHeight()
			[
				CreateForceReassignRenderMaterialsGui()
			]
		];
	// clang-format on
}

TSharedRef<SBorder> SAGX_SynchronizeModelDialog::CreateSynchronizeButtonGui()
{
	if (ImportType == EAGX_ImportType::Invalid)
	{
		return MakeShared<SBorder>();
	}

	// clang-format off
	return SNew(SBorder)
				.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f))
				.BorderImage(FAGX_EditorUtilities::GetBrush("ToolPanel.GroupBorder"))
				.Padding(FMargin(5.0f, 5.0f))
				.Content()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.Text(LOCTEXT("SynchronizeButtonText", "Synchronize Model"))
						.ToolTipText(LOCTEXT("SynchronizeButtonTooltip",
							"Synchronizes the model against the source file of the original "
							"import and updates the Components and Assets to match said source file."))
						.OnClicked(this, &SAGX_SynchronizeModelDialog::OnSynchronizeButtonClicked)
					]
				];

	// clang-format on
}

TSharedRef<SWidget> SAGX_SynchronizeModelDialog::CreateForceOverwritePropertiesGui()
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
					.ToolTipText(LOCTEXT("ForceOverwritePropertiesTooltip",
						"Properties that has been changed by the user in Unreal will be overwritten unconditionally. "
						"If left unchecked, properties changed by the user in Unreal will be preserved."))
					.OnCheckStateChanged(this, &SAGX_SynchronizeModelDialog::OnForceOverwritePropertiesClicked)
					.IsChecked(bForceOverwriteProperties)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(0.f, 0.f, 33.f, 0.f))
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ForceOverwritePropertiesText", "Force overwrite properties"))
				.Font(FAGX_SlateUtilities::CreateFont(10))
			]
		];
	// clang-format on
}

TSharedRef<SWidget> SAGX_SynchronizeModelDialog::CreateForceReassignRenderMaterialsGui()
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
					.ToolTipText(LOCTEXT("ForceReassignRenderMaterialsTooltip",
						"Render Materials that has been assigned by the user in Unreal will be re-assigned unconditionally. "
						"If left unchecked, render Materials assigned by the user in Unreal will be preserved."))
					.OnCheckStateChanged(this, &SAGX_SynchronizeModelDialog::OnForceReassignRenderMaterialsClicked)
					.IsChecked(bForceReassignRenderMaterials)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(0.f, 0.f, 33.f, 0.f))
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ForceReassignRenderMaterialsText", "Force re-assign render Materials"))
				.Font(FAGX_SlateUtilities::CreateFont(10))
			]
		];
	// clang-format on
}

FReply SAGX_SynchronizeModelDialog::OnSynchronizeButtonClicked()
{
	bUserHasPressedImportOrSynchronize = true;

	// We are done, close the Window containing this Widget. The user of this Widget should get
	// the user's input via the ToImportSettings function when the Window has closed.
	TSharedRef<SWindow> ParentWindow =
		FSlateApplication::Get().FindWidgetWindow(AsShared()).ToSharedRef();
	FSlateApplication::Get().RequestDestroyWindow(ParentWindow);

	return FReply::Handled();
}

void SAGX_SynchronizeModelDialog::OnForceOverwritePropertiesClicked(ECheckBoxState NewCheckedState)
{
	bForceOverwriteProperties = NewCheckedState == ECheckBoxState::Checked;
}

void SAGX_SynchronizeModelDialog::OnForceReassignRenderMaterialsClicked(
	ECheckBoxState NewCheckedState)
{
	bForceReassignRenderMaterials = NewCheckedState == ECheckBoxState::Checked;
}

#undef LOCTEXT_NAMESPACE
