// Copyright 2024, Algoryx Simulation AB.

#include "OpenPLX/PLX_SignalHandlerComponentCustomization.h"

// AGX Dynamics for Unreal includes.
#include "OpenPLX/PLX_SignalHandlerComponent.h"
#include "Utilities/AGX_EditorUtilities.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FPLX_SignalHandlerComponentCustomization"

TSharedRef<IDetailCustomization> FPLX_SignalHandlerComponentCustomization::MakeInstance()
{
	return MakeShareable(new FPLX_SignalHandlerComponentCustomization);
}

void FPLX_SignalHandlerComponentCustomization::CustomizeDetails(
	IDetailLayoutBuilder& InDetailBuilder)
{
	UPLX_SignalHandlerComponent* Component =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UPLX_SignalHandlerComponent>(
			InDetailBuilder);
	if (Component == nullptr)
		return;

	InDetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UPLX_SignalHandlerComponent, Outputs));
	InDetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UPLX_SignalHandlerComponent, Inputs));
	InDetailBuilder.HideCategory(FName("Sockets"));

	IDetailCategoryBuilder& SignalInterfaceCategory =
		InDetailBuilder.EditCategory("OpenPLX Signal Interface", FText::GetEmpty(), ECategoryPriority::Important);

	// clang-format off

	// SignalInterface Inputs.
	SignalInterfaceCategory.AddCustomRow(FText::FromString("Signal Interface Inputs"))
	.WholeRowContent()
	[
		SNew(STextBlock)
		.Text(FText::FromString("Signal Interface Inputs"))
		.Font(IDetailLayoutBuilder::GetDetailFontBold())
	];

	TArray<FString> SortedInputAliasKeys;
	Component->InputAliases.GetKeys(SortedInputAliasKeys);
	SortedInputAliasKeys.Sort();
	for (const FString& Alias : SortedInputAliasKeys)
	{
		const FString& Key = Component->InputAliases[Alias];
		const FPLX_Input* Input = Component->Inputs.Find(Key);
		if (!Input)
			continue;

		FString InputTypeName = UEnum::GetValueAsString(Input->Type);
		InputTypeName = InputTypeName.RightChop(InputTypeName.Find(TEXT("::")) + 2);

		SignalInterfaceCategory.AddCustomRow(FText::FromString("InputExpandable"))
		.WholeRowContent()
		[
			SNew(SExpandableArea)
			.InitiallyCollapsed(false)
			.HeaderContent()
			[
				SNew(SEditableTextBox)
				.Text(FText::FromString(Alias))
				.IsReadOnly(true)
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
			.BodyContent()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(32.f, 0.f, 0.f, 0.f))
				[
					SNew(SEditableTextBox)
					.Text(FText::FromString(FString::Printf(TEXT("Type: %s"), *InputTypeName)))
					.IsReadOnly(true)
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(32.f, 4.f, 32.f, 4.f)
				[
					SNew(SEditableTextBox)
					.Text(FText::FromString(FString::Printf(TEXT("Full name: %s"), *Input->Name)))
					.IsReadOnly(true)
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			]
		];
	}

	// SignalInterface Outputs.
	SignalInterfaceCategory.AddCustomRow(FText::FromString("Signal Interface Outputs"))
	.WholeRowContent()
	[
		SNew(STextBlock)
		.Text(FText::FromString("Signal Interface Outputs"))
		.Font(IDetailLayoutBuilder::GetDetailFontBold())
	];

	TArray<FString> SortedOutputAliasKeys;
	Component->OutputAliases.GetKeys(SortedOutputAliasKeys);
	SortedOutputAliasKeys.Sort();
	for (const FString& Alias : SortedOutputAliasKeys)
	{
		const FString& Key = Component->OutputAliases[Alias];
		const FPLX_Output* Output = Component->Outputs.Find(Key);
		if (!Output)
			continue;

		if (!Component->bShowDisabledOutputs && !Output->bEnabled)
			continue;

		FString OutputTypeName = UEnum::GetValueAsString(Output->Type);
		OutputTypeName = OutputTypeName.RightChop(OutputTypeName.Find(TEXT("::")) + 2);

		SignalInterfaceCategory.AddCustomRow(FText::FromString("OutputExpandable"))
		.WholeRowContent()
		[
			SNew(SExpandableArea)
			.InitiallyCollapsed(true)
			.HeaderContent()
			[
				SNew(SEditableTextBox)
				.Text(FText::FromString(Alias))
				.IsReadOnly(true)
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
			.BodyContent()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(32.f, 0.f, 0.f, 0.f))
				[
					SNew(SEditableTextBox)
					.Text(FText::FromString(FString::Printf(TEXT("Type: %s"), *OutputTypeName)))
					.IsReadOnly(true)
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(32.f, 4.f, 32.f, 4.f)
				[
					SNew(SEditableTextBox)
					.Text(FText::FromString(Output->bEnabled ? TEXT("Enabled") : TEXT("Disabled")))
					.IsReadOnly(true)
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(32.f, 4.f, 32.f, 4.f)
				[
					SNew(SEditableTextBox)
					.Text(FText::FromString(FString::Printf(TEXT("Full name: %s"), *Output->Name)))
					.IsReadOnly(true)
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			]
		];
	}

	IDetailCategoryBuilder& InputsCategory =
		InDetailBuilder.EditCategory("OpenPLX Inputs", FText::GetEmpty(), ECategoryPriority::Important);
	InputsCategory.InitiallyCollapsed(true);

  // All Inputs.
	InputsCategory.AddCustomRow(FText::FromString("All Inputs"))
	.WholeRowContent()
	[
		SNew(STextBlock)
		.Text(FText::FromString("Inputs"))
		.Font(IDetailLayoutBuilder::GetDetailFontBold())
	];

	TArray<FString> SortedInputKeys;
	Component->Inputs.GetKeys(SortedInputKeys);
	SortedInputKeys.Sort();
	for (const FString& Key : SortedInputKeys)
	{
		const FPLX_Input& Input = Component->Inputs[Key];
		FString InputTypeName = UEnum::GetValueAsString(Input.Type);
		InputTypeName = InputTypeName.RightChop(InputTypeName.Find(TEXT("::")) + 2);

		InputsCategory.AddCustomRow(FText::FromString("InputExpandable"))
		.WholeRowContent()
		[
			SNew(SExpandableArea)
			.InitiallyCollapsed(true)
			.HeaderContent()
			[
				SNew(SEditableTextBox)
				.Text(FText::FromString(Key))
				.IsReadOnly(true)
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
			.BodyContent()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(32.f, 0.f, 0.f, 0.f))
				[
					SNew(SEditableTextBox)
					.Text(FText::FromString(FString::Printf(TEXT("Type: %s"), *InputTypeName)))
					.IsReadOnly(true)
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(32.f, 4.f, 0.f, 0.f))
				[
					SNew(SBox)
					.Visibility(Input.Alias.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible)
					[
						SNew(SEditableTextBox)
						.Text(FText::FromString(FString::Printf(TEXT("Alias: %s"), *Input.Alias)))
						.IsReadOnly(true)
						.Font(IDetailLayoutBuilder::GetDetailFont())
					]
				]
			]
		];
	}

	IDetailCategoryBuilder& OutputsCategory =
		InDetailBuilder.EditCategory("OpenPLX Outputs", FText::GetEmpty(), ECategoryPriority::Important);
	OutputsCategory.InitiallyCollapsed(true);

	// Hide disabled Outputs checkbox.
	OutputsCategory.AddCustomRow(FText::FromString("OutputsHeader"))
	.WholeRowContent()
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString("All Outputs"))
			.Font(IDetailLayoutBuilder::GetDetailFontBold())
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.f, 4.f)
		[
			SNew(SCheckBox)
			.IsChecked_Lambda([Component]() { return Component->bShowDisabledOutputs ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
			.OnCheckStateChanged_Lambda([Component, &InDetailBuilder](ECheckBoxState NewState)
			{
				Component->bShowDisabledOutputs = (NewState == ECheckBoxState::Checked);
				FSlateApplication::Get().DismissAllMenus();
				InDetailBuilder.ForceRefreshDetails();
			})
			[
				SNew(STextBlock)
				.Text(FText::FromString("Show Disabled Outputs"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
		]
	];

	// All Outputs.
	TArray<FString> SortedOutputKeys;
	Component->Outputs.GetKeys(SortedOutputKeys);
	SortedOutputKeys.Sort();
	for (const FString& Key : SortedOutputKeys)
	{
		const FPLX_Output& Output = Component->Outputs[Key];
		if (!Component->bShowDisabledOutputs && !Output.bEnabled)
			continue;

		FString OutputTypeName = UEnum::GetValueAsString(Output.Type);
		OutputTypeName = OutputTypeName.RightChop(OutputTypeName.Find(TEXT("::")) + 2);

		OutputsCategory.AddCustomRow(FText::FromString("OutputExpandable"))
		.WholeRowContent()
		[
			SNew(SExpandableArea)
			.InitiallyCollapsed(true)
			.HeaderContent()
			[
				SNew(SEditableTextBox)
				.Text(FText::FromString(Key))
				.IsReadOnly(true)
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
			.BodyContent()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(32.f, 0.f, 0.f, 0.f))
				[
					SNew(SEditableTextBox)
					.Text(FText::FromString(FString::Printf(TEXT("Type: %s"), *OutputTypeName)))
					.IsReadOnly(true)
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(32.f, 4.f, 32.f, 4.f)
				[
					SNew(SEditableTextBox)
					.Text(FText::FromString(Output.bEnabled ? TEXT("Enabled") : TEXT("Disabled")))
					.IsReadOnly(true)
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(32.f, 4.f, 0.f, 0.f))
				[
					SNew(SBox)
					.Visibility(Output.Alias.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible)
					[
						SNew(SEditableTextBox)
						.Text(FText::FromString(FString::Printf(TEXT("Alias: %s"), *Output.Alias)))
						.IsReadOnly(true)
						.Font(IDetailLayoutBuilder::GetDetailFont())
					]
				]
			]
		];
	}
	// clang-format on
}

#undef LOCTEXT_NAMESPACE
