// Copyright 2025, Algoryx Simulation AB.

#if AGXUNREAL_USE_OPENPLX

#include "OpenPLX/OpenPLX_SignalHandlerComponentCustomization.h"

// AGX Dynamics for Unreal includes.
#include "OpenPLX/OpenPLX_SignalHandlerComponent.h"
#include "Utilities/AGX_EditorUtilities.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateTypes.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FOpenPLX_SignalHandlerComponentCustomization"

TSharedRef<IDetailCustomization> FOpenPLX_SignalHandlerComponentCustomization::MakeInstance()
{
	return MakeShareable(new FOpenPLX_SignalHandlerComponentCustomization);
}

void FOpenPLX_SignalHandlerComponentCustomization::CustomizeDetails(
	IDetailLayoutBuilder& InDetailBuilder)
{
	UOpenPLX_SignalHandlerComponent* Component =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UOpenPLX_SignalHandlerComponent>(
			InDetailBuilder);
	if (Component == nullptr)
		return;

	InDetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UOpenPLX_SignalHandlerComponent, Outputs));
	InDetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UOpenPLX_SignalHandlerComponent, Inputs));
	InDetailBuilder.HideCategory(FName("Sockets"));

	IDetailCategoryBuilder& SignalInterfaceCategory = InDetailBuilder.EditCategory(
		"OpenPLX Signal Interface", FText::GetEmpty(), ECategoryPriority::Important);

	// clang-format off

	// SignalInterface Inputs.
	SignalInterfaceCategory.AddCustomRow(FText::FromString("Signal Interface Inputs"))
	.WholeRowContent()
	[
		SNew(STextBlock)
		.Text(FText::FromString("Signal Interface Inputs"))
		.Font(IDetailLayoutBuilder::GetDetailFontBold())
	];

	TArray<FName> SortedInputAliasKeys;
	Component->InputAliases.GetKeys(SortedInputAliasKeys);
	SortedInputAliasKeys.Sort([](const FName& A, const FName& B)
	{
		return A.LexicalLess(B);
	});

	for (const FName& Alias : SortedInputAliasKeys)
	{
		const FName& Key = Component->InputAliases[Alias];
		const FOpenPLX_Input* Input = Component->Inputs.Find(Key);
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
				.Text(FText::FromName(Alias))
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
					.Text(FText::FromString(FString::Printf(TEXT("Full name: %s"), *Input->Name.ToString())))
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

	TArray<FName> SortedOutputAliasKeys;
	Component->OutputAliases.GetKeys(SortedOutputAliasKeys);
	SortedOutputAliasKeys.Sort([](const FName& A, const FName& B)
	{
		return A.LexicalLess(B);
	});

	for (const FName& Alias : SortedOutputAliasKeys)
	{
		const FName& Key = Component->OutputAliases[Alias];
		const FOpenPLX_Output* Output = Component->Outputs.Find(Key);
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
				.Text(FText::FromName(Alias))
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
					.Text(FText::FromString(FString::Printf(TEXT("Full name: %s"), *Output->Name.ToString())))
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

	TArray<FName> SortedInputKeys;
	Component->Inputs.GetKeys(SortedInputKeys);
	SortedInputKeys.Sort([](const FName& A, const FName& B)
	{
		return A.LexicalLess(B);
	});

	for (const FName& Key : SortedInputKeys)
	{
		const FOpenPLX_Input& Input = Component->Inputs[Key];
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
				.Text(FText::FromName(Key))
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
					.Visibility(Input.Alias.IsNone() ? EVisibility::Collapsed : EVisibility::Visible)
					[
						SNew(SEditableTextBox)
						.Text(FText::FromString(FString::Printf(TEXT("Alias: %s"), *Input.Alias.ToString())))
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
	TArray<FName> SortedOutputKeys;
	Component->Outputs.GetKeys(SortedOutputKeys);
	SortedOutputKeys.Sort([](const FName& A, const FName& B)
	{
		return A.LexicalLess(B);
	});

	for (const FName& Key : SortedOutputKeys)
	{
		const FOpenPLX_Output& Output = Component->Outputs[Key];
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
				.Text(FText::FromName(Key))
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
					.Visibility(Output.Alias.IsNone() ? EVisibility::Collapsed : EVisibility::Visible)
					[
						SNew(SEditableTextBox)
						.Text(FText::FromString(FString::Printf(TEXT("Alias: %s"), *Output.Alias.ToString())))
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

#endif
