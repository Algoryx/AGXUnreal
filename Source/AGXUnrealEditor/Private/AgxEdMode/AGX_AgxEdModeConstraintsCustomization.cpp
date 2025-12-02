// Copyright 2025, Algoryx Simulation AB.

#include "AgxEdMode/AGX_AgxEdModeConstraintsCustomization.h"

// AGX Dynamics for Unreal includes.
#include "AgxEdMode/AGX_AgxEdModeConstraints.h"
#include "Constraints/AGX_ConstraintActor.h"
#include "Constraints/AGX_ConstraintComponent.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_PropertyUtilities.h"

// Unreal Engine includes.
#if UE_VERSION_OLDER_THAN(5, 0, 0) == false
#include "ActorMode.h"
#include "ActorTreeItem.h"
#endif
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "EditorStyleSet.h"
#include "IDetailPropertyRow.h"
#include "Misc/Attribute.h"
#include "Modules/ModuleManager.h"
#include "SceneOutlinerFilters.h"
#include "SceneOutlinerModule.h"
#include "SceneOutlinerPublicTypes.h"
#include "UObject/MetaData.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FAGX_AgxEdModeConstraintsCustomization"

TSharedRef<IDetailCustomization> FAGX_AgxEdModeConstraintsCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_AgxEdModeConstraintsCustomization);
}

FAGX_AgxEdModeConstraintsCustomization::FAGX_AgxEdModeConstraintsCustomization()
{
	// Find all spawnable constraint classes so that they can be presented by combo box.
	FAGX_EditorUtilities::GetAllClassesOfType(
		ConstraintClasses, AAGX_ConstraintActor::StaticClass(),
		/*bIncludeAbstract*/ false);

	UClass* Class = StaticClass<AAGX_ConstraintActor>();
}

void FAGX_AgxEdModeConstraintsCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	UAGX_AgxEdModeConstraints* ConstraintsSubMode =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_AgxEdModeConstraints>(
			DetailBuilder);

	if (!ConstraintsSubMode)
		return;

	CreateConstraintCreatorCategory(DetailBuilder, ConstraintsSubMode);

	CreateConstraintBrowserCategory(DetailBuilder, ConstraintsSubMode);
}

void FAGX_AgxEdModeConstraintsCustomization::CreateConstraintCreatorCategory(
	IDetailLayoutBuilder& DetailBuilder, UAGX_AgxEdModeConstraints* ConstraintsSubMode)
{
	check(ConstraintsSubMode);

	IDetailCategoryBuilder& CategoryBuilder = DetailBuilder.EditCategory("Constraint Creator");

	CategoryBuilder.InitiallyCollapsed(true);

	/** Constraint Type - Combo Box */
	CreateConstraintTypeComboBox(CategoryBuilder, ConstraintsSubMode);

	/** Rigid Body 1 - Picker */
	CategoryBuilder.AddProperty(
		DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAGX_AgxEdModeConstraints, RigidBody1)));

	/** Rigid Body 2 - Picker */
	CategoryBuilder.AddProperty(
		DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAGX_AgxEdModeConstraints, RigidBody2)));

	/** Find Actors From Selection - Button */
	CreateGetFromSelectedActorsButton(CategoryBuilder, ConstraintsSubMode);

	/** Constraint Parent - Combo Box */
	CategoryBuilder.AddProperty(DetailBuilder.GetProperty(
		GET_MEMBER_NAME_CHECKED(UAGX_AgxEdModeConstraints, ConstraintParent)));

	/** Frame Source - Radio Buttons */
	CreateFrameSourceRadioButtons(CategoryBuilder, ConstraintsSubMode);

	/** Create Constraint Button */
	CategoryBuilder.AddCustomRow(FText::GetEmpty())
		[SNew(SHorizontalBox) + SHorizontalBox::Slot().FillWidth(1)

		 + SHorizontalBox::Slot().AutoWidth()[SNew(SButton)
												  .Text(LOCTEXT("CreateButtonText", "Create"))
												  .OnClicked_Lambda(
													  [ConstraintsSubMode]()
													  {
														  if (ConstraintsSubMode)
															  ConstraintsSubMode
																  ->CreateConstraint();
														  return FReply::Handled();
													  })]];
}

void FAGX_AgxEdModeConstraintsCustomization::CreateConstraintTypeComboBox(
	IDetailCategoryBuilder& CategoryBuilder, UAGX_AgxEdModeConstraints* ConstraintsSubMode)
{
	// Set initial selection, if necessary.
	if (ConstraintsSubMode->ConstraintType == nullptr && ConstraintClasses.Num() > 0)
	{
		ConstraintsSubMode->ConstraintType = ConstraintClasses[0];
	}

	// Function for getting a shorter display name per constraint type
	auto GetShortName = [](UClass* ConstraintClass) -> FText
	{
		if (!ConstraintClass)
		{
			return FText::GetEmpty();
		}

		FString DisplayName = FAGX_PropertyUtilities::GetActualDisplayName(
			ConstraintClass, /*bRemoveAgxPrefix*/ true);
		DisplayName.RemoveFromEnd(" Constraint", ESearchCase::CaseSensitive);
		return FText::FromString(DisplayName);
	};

	auto GetToolTip = [](UClass* ConstraintClass) -> FText
	{
		if (!ConstraintClass)
		{
			return FText::GetEmpty();
		}

		return ConstraintClass->GetToolTipText(/*bShortTooltip*/ true);
	};

	CategoryBuilder.AddCustomRow(FText::GetEmpty())
		.NameContent()[SNew(STextBlock)
						   .Text(LOCTEXT("ConstraintType", "Constraint Type"))
						   .Font(IDetailLayoutBuilder::GetDetailFont())]
		.ValueContent()
			[SNew(SComboBox<UClass*>)
				 .ContentPadding(2)
				 .OptionsSource(&ConstraintClasses)
				 .OnGenerateWidget_Lambda(
					 [=](UClass* Item) // content for each item in combo box
					 {
						 return SNew(STextBlock)
							 .Text(GetShortName(Item))
							 .ToolTipText(GetToolTip(Item));
					 })
				 .OnSelectionChanged(
					 this, &FAGX_AgxEdModeConstraintsCustomization::OnConstraintTypeComboBoxChanged,
					 ConstraintsSubMode)
				 .ToolTipText_Lambda(
					 [=]()
					 { return GetToolTip(ConstraintsSubMode->ConstraintType); }) // header tooltip
				 .Content() // header content (i.e. showing selected item, even while combo box is
							// closed)
					 [SNew(STextBlock)
						  .Text_Lambda([=]()
									   { return GetShortName(ConstraintsSubMode->ConstraintType); })
						  .Font(IDetailLayoutBuilder::GetDetailFont())]];
}

void FAGX_AgxEdModeConstraintsCustomization::CreateGetFromSelectedActorsButton(
	IDetailCategoryBuilder& CategoryBuilder, UAGX_AgxEdModeConstraints* ConstraintsSubMode)
{
	// clang-format off
	CategoryBuilder.AddCustomRow(FText::GetEmpty())
	.ValueContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.HAlign(HAlign_Left)
		//.Padding(5, 0)
		[
			SNew(SButton)
			.Text(LOCTEXT("FindActorsFromSelectionButtonText", "Get Actors From Selection"))
			.ToolTipText(
				LOCTEXT(
					"FindActorsFromSelectionButtonTooltip",
					"Searches selected actors for two Actors with a Rigid Body Component."
					"\n\n"
					"Also searches the subtree and parent chain of each selected actor if a direct "
					"match was not found."))
		  	.OnClicked_Lambda([ConstraintsSubMode]() {
				AActor* Actor1;
				AActor* Actor2;
				FAGX_EditorUtilities::GetRigidBodyActorsFromSelection(
					&Actor1, &Actor2, /*bSearchSubtrees*/ true, /*bSearchAncestors*/ true);
				ConstraintsSubMode->RigidBody1.OwningActor = Actor1;
				ConstraintsSubMode->RigidBody2.OwningActor = Actor2;
				return FReply::Handled();
			})
		]
	];
	// clang-format on
}

void FAGX_AgxEdModeConstraintsCustomization::CreateFrameSourceRadioButtons(
	IDetailCategoryBuilder& CategoryBuilder, UAGX_AgxEdModeConstraints* ConstraintsSubMode)
{
	check(ConstraintsSubMode);

	const TSharedRef<IPropertyHandle> PropertyHandle =
		CategoryBuilder.GetParentLayout().GetProperty(
			GET_MEMBER_NAME_CHECKED(UAGX_AgxEdModeConstraints, AttachmentFrameSource));

	UEnum* FrameSourceEnum = StaticEnum<EAGX_ConstraintCreationFrameSource>();
	check(FrameSourceEnum);

	IDetailPropertyRow& PropertyRow = CategoryBuilder.AddProperty(PropertyHandle);

	TSharedPtr<SWidget> DefaultNameWidget;
	TSharedPtr<SWidget> DefaultValueWidget;
	PropertyRow.GetDefaultWidgets(
		DefaultNameWidget, DefaultValueWidget, /*bAddWidgetDecoration*/ true);

	const TSharedRef<SVerticalBox> RadioButtonsBox = SNew(SVerticalBox);

	PropertyRow.CustomWidget(/*bShowChildren*/ true)
		[SNew(SVerticalBox)

		 /** Label for property name */
		 + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left)[DefaultNameWidget.ToSharedRef()]

		 /** Area for the radio buttons */
		 + SVerticalBox::Slot()
			   .AutoHeight()
			   .HAlign(HAlign_Center)
			   .Padding(FMargin(5.0f, 0.0f))[RadioButtonsBox]];

	// Fill with one radio button for each enum value
	const int32 NumEnums =
		FrameSourceEnum->NumEnums() - 1; // minus one because Unreal adds a "MAX" item on the end
	for (int32 EnumIndex = 0; EnumIndex < NumEnums; ++EnumIndex)
	{
		const EAGX_ConstraintCreationFrameSource EnumValue =
			static_cast<EAGX_ConstraintCreationFrameSource>(
				FrameSourceEnum->GetValueByIndex(EnumIndex));

		RadioButtonsBox->AddSlot().Padding(FMargin(
			0.0f,
			1.0f))[SNew(SCheckBox)
					   .Style(FAGX_EditorUtilities::GetStyle(), "RadioButton")
					   .Padding(FMargin(4.0f, 0.0f))
					   .ToolTipText(FrameSourceEnum->GetToolTipTextByIndex(EnumIndex))
					   .IsChecked_Lambda(
						   [ConstraintsSubMode, EnumValue]()
						   {
							   return ConstraintsSubMode->AttachmentFrameSource == EnumValue
										  ? ECheckBoxState::Checked
										  : ECheckBoxState::Unchecked;
						   })
					   .OnCheckStateChanged(
						   this,
						   &FAGX_AgxEdModeConstraintsCustomization::OnFrameSourceRadioButtonChanged,
						   EnumValue, ConstraintsSubMode)
						   [SNew(STextBlock)
								.Text(FrameSourceEnum->GetDisplayNameTextByIndex(EnumIndex))
								.Font(IDetailLayoutBuilder::GetDetailFont())]];
	}
}

void FAGX_AgxEdModeConstraintsCustomization::OnConstraintTypeComboBoxChanged(
	UClass* NewSelectedItem, ESelectInfo::Type InSeletionInfo,
	UAGX_AgxEdModeConstraints* ConstraintsSubMode)
{
	if (ConstraintsSubMode)
	{
		ConstraintsSubMode->ConstraintType = NewSelectedItem;
	}
}

void FAGX_AgxEdModeConstraintsCustomization::OnFrameSourceRadioButtonChanged(
	ECheckBoxState NewCheckedState, EAGX_ConstraintCreationFrameSource RadioButton,
	UAGX_AgxEdModeConstraints* ConstraintsSubMode)
{
	if (ConstraintsSubMode && NewCheckedState == ECheckBoxState::Checked)
	{
		ConstraintsSubMode->AttachmentFrameSource = RadioButton;
	}
}

void FAGX_AgxEdModeConstraintsCustomization::CreateConstraintBrowserCategory(
	IDetailLayoutBuilder& DetailBuilder, UAGX_AgxEdModeConstraints* ConstraintsSubMode)
{
	check(ConstraintsSubMode);

	IDetailCategoryBuilder& CategoryBuilder = DetailBuilder.EditCategory("Constraint Browser");

	CategoryBuilder.InitiallyCollapsed(false);

	CreateConstraintBrowserListView(CategoryBuilder, ConstraintsSubMode);
}

void FAGX_AgxEdModeConstraintsCustomization::CreateConstraintBrowserListView(
	IDetailCategoryBuilder& CategoryBuilder, UAGX_AgxEdModeConstraints* ConstraintsSubMode)
{
	auto ContainsConstraint = [](const AActor* Actor)
	{
		if (Actor == nullptr)
		{
			return false;
		}

		return Actor->FindComponentByClass<UAGX_ConstraintComponent>() != nullptr;
	};

#if UE_VERSION_OLDER_THAN(5, 0, 0)
	using namespace SceneOutliner;

	FSceneOutlinerModule& SceneOutlinerModule =
		FModuleManager::LoadModuleChecked<FSceneOutlinerModule>("SceneOutliner");

	FActorFilterPredicate ActorFilter = FActorFilterPredicate::CreateLambda(ContainsConstraint);

	FCustomSceneOutlinerDeleteDelegate DeleteAction =
		FCustomSceneOutlinerDeleteDelegate::CreateLambda(
			[](const TArray<TWeakObjectPtr<AActor>>& Actors) {}); // no delete

	FInitializationOptions Options;
	Options.Mode = ESceneOutlinerMode::ActorBrowsing;
	Options.bShowHeaderRow = true;
	Options.bShowParentTree = false;
	Options.bShowSearchBox = false;
	Options.bFocusSearchBoxWhenOpened = false;
	Options.bShowTransient = true;
	Options.bShowCreateNewFolder = false;
	Options.ColumnMap.Add(FBuiltInColumnTypes::Label(), FColumnInfo(EColumnVisibility::Visible, 0));
	Options.ColumnMap.Add(
		FBuiltInColumnTypes::ActorInfo(), FColumnInfo(EColumnVisibility::Visible, 10));
	Options.CustomDelete = DeleteAction;
	Options.Filters->AddFilterPredicate(ActorFilter);

	TSharedRef<SWidget> SceneOutliner =
		SceneOutlinerModule.CreateSceneOutliner(Options, FOnActorPicked());

	CategoryBuilder.AddCustomRow(FText::GetEmpty())
		.WholeRowContent()[SNew(SScrollBox) + SScrollBox::Slot().Padding(0)[SceneOutliner]];
#else
	FSceneOutlinerModule& SceneOutlinerModule =
		FModuleManager::LoadModuleChecked<FSceneOutlinerModule>("SceneOutliner");

	using FActorFilter = TSceneOutlinerPredicateFilter<FActorTreeItem>;
	TSharedPtr<FActorFilter> ActorFilter = MakeShared<FActorFilter>(
		FActorTreeItem::FFilterPredicate::CreateLambda(ContainsConstraint),
		FSceneOutlinerFilter::EDefaultBehaviour::Fail);

	FCustomSceneOutlinerDeleteDelegate DeleteAction =
		FCustomSceneOutlinerDeleteDelegate::CreateLambda(
			[](const TArray<TWeakPtr<ISceneOutlinerTreeItem>>& Items) {}); // no delete

	FSceneOutlinerInitializationOptions Options;
	Options.ModeFactory = FCreateSceneOutlinerMode::CreateLambda(
		[](SSceneOutliner* Outliner) { return new FActorMode(Outliner); });
	Options.bShowHeaderRow = true;
	Options.bShowParentTree = false;
	Options.bShowSearchBox = false;
	Options.bFocusSearchBoxWhenOpened = false;
	Options.bShowTransient = true;
	Options.bShowCreateNewFolder = false;
	Options.ColumnMap.Add(FSceneOutlinerBuiltInColumnTypes::Label(), FSceneOutlinerColumnInfo());
	Options.ColumnMap[FSceneOutlinerBuiltInColumnTypes::Label()].ColumnLabel =
		FSceneOutlinerBuiltInColumnTypes::Label_Localized();
	Options.ColumnMap.Add(
		FSceneOutlinerBuiltInColumnTypes::ActorInfo(), FSceneOutlinerColumnInfo());
	Options.ColumnMap[FSceneOutlinerBuiltInColumnTypes::ActorInfo()].ColumnLabel =
		FSceneOutlinerBuiltInColumnTypes::ActorInfo_Localized();
	Options.CustomDelete = DeleteAction;
	Options.Filters->Add(ActorFilter);

	TSharedRef<SWidget> SceneOutliner = SceneOutlinerModule.CreateSceneOutliner(Options);

	CategoryBuilder.AddCustomRow(FText::GetEmpty())
		.WholeRowContent()[SNew(SScrollBox) + SScrollBox::Slot().Padding(0)[SceneOutliner]];
#endif
}

#undef LOCTEXT_NAMESPACE
