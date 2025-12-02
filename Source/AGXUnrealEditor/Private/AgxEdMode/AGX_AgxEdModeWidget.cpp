// Copyright 2025, Algoryx Simulation AB.

#include "AgxEdMode/AGX_AgxEdModeWidget.h"

// AGX Dynamics for Unreal includes.
#include "AGX_EditorStyle.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "AgxEdMode/AGX_AgxEdModeSubMode.h"
#include "AgxEdMode/AGX_AgxEdMode.h"

// Unreal Engine includes.
#include "Framework/Application/SlateApplication.h"
#include "EditorFontGlyphs.h"
#include "IDetailsView.h"
#include "IPropertyTypeCustomization.h"
#include "Misc/EngineVersionComparison.h"
#include "Modules/ModuleManager.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "PropertyEditorModule.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SHeader.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "SAGX_AgxEdModeWidget"

void SAGX_AgxEdModeWidget::Construct(const FArguments& InArgs, FAGX_AgxEdMode* InAgxEdMode)
{
	AgxEdMode = InAgxEdMode;

	auto GetCurrentSubModeName = [AgxEdMode = AgxEdMode]()
	{
		check(AgxEdMode);
		UAGX_AgxEdModeSubMode* SubMode = AgxEdMode->GetCurrentSubMode();
		return SubMode ? SubMode->GetDisplayName() : FText::GetEmpty();
	};

	// clang-format off
	ChildSlot
	[
		SNew(SScrollBox)
		+ SScrollBox::Slot().Padding(0.0f)
		[
			SNew(SVerticalBox)
			/** Toolbar buttons for switching between sub-modes */
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SBorder)
				.BorderImage(FAGX_EditorUtilities::GetBrush("ToolPanel.GroupBorder"))
				.HAlign(HAlign_Center)
				[
					CreateSubModesToolbar()
				]
			]
			/** Header for the current sub-mode */
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(5.0f, 10.0f))
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SNew(SHeader)
				[
					SNew(STextBlock)
					.Text_Lambda(GetCurrentSubModeName)
					.TextStyle(FAGX_EditorUtilities::GetStyle(),"ContentBrowser.TopBar.Font")
				]
			]
			/** DetailsView for the current sub-mode */
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				CreateSubModeDetailsView()
			]
		]
	];
	// clang-format on
}

void SAGX_AgxEdModeWidget::OnSubModeChanged()
{
	// Change Details View's display-object to the current Sub Mode.
	if (SubModeDetailsView)
	{
		TArray<UObject*> PropertyObjects;

		if (UObject* CurrentSubMode = AgxEdMode->GetCurrentSubMode())
		{
			PropertyObjects.Add(CurrentSubMode);
		}

		SubModeDetailsView->SetObjects(PropertyObjects);
	}
}

TSharedRef<IDetailsView> SAGX_AgxEdModeWidget::CreateSubModeDetailsView()
{
	FDetailsViewArgs Args;
	Args.bUpdatesFromSelection = false;
	Args.bLockable = false;
	Args.bHideSelectionTip = true; /// \todo True or false?
	Args.bAllowSearch = false;
#if UE_VERSION_OLDER_THAN(5, 0, 0)
	Args.bShowActorLabel = false;
#else
	Args.bShowObjectLabel = false;
#endif
	Args.NameAreaSettings = FDetailsViewArgs::HideNameArea;

	FPropertyEditorModule& PropertyModule =
		FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	SubModeDetailsView = PropertyModule.CreateDetailView(Args);

	OnSubModeChanged();

	return SubModeDetailsView.ToSharedRef();
}

TSharedRef<SWidget> SAGX_AgxEdModeWidget::CreateSubModesToolbar()
{
	FToolBarBuilder ToolBar(MakeShareable(new FUICommandList()), FMultiBoxCustomization::None);

	ToolBar.SetLabelVisibility(EVisibility::Visible);

	if (AgxEdMode)
	{
		const TArray<UAGX_AgxEdModeSubMode*>& SubModes = AgxEdMode->GetSubModes();

		for (UAGX_AgxEdModeSubMode* SubMode : SubModes)
		{
			TAttribute<FSlateIcon> SubModeIcon;
			SubModeIcon.Bind(TAttribute<FSlateIcon>::FGetter::CreateLambda(
				[SubMode]() { return SubMode->GetIcon(); }));

			ToolBar.AddToolBarButton(
				FUIAction(
					FExecuteAction::CreateRaw(
						AgxEdMode, &FAGX_AgxEdMode::SetCurrentSubMode, SubMode),
					FCanExecuteAction(),
					FIsActionChecked::CreateLambda(
						[AgxEdMode = AgxEdMode, SubMode]()
						{ return AgxEdMode->GetCurrentSubMode() == SubMode; })),
				NAME_None, SubMode->GetDisplayName(), SubMode->GetTooltip(), SubModeIcon,
				EUserInterfaceActionType::RadioButton,
				/*InTutorialHighlightName*/ NAME_None);
		}
	}

	return ToolBar.MakeWidget();
}

#undef LOCTEXT_NAMESPACE
