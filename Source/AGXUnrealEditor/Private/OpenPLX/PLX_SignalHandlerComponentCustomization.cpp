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

	IDetailCategoryBuilder& CategoryBuilder =
		InDetailBuilder.EditCategory("OpenPLX", FText::GetEmpty(), ECategoryPriority::Important);

	// clang-format off

  // Inputs.
	CategoryBuilder.AddCustomRow(FText::FromString("Inputs"))
	.WholeRowContent()
	[
		SNew(STextBlock)
		.Text(FText::FromString("Inputs"))
		.Font(IDetailLayoutBuilder::GetDetailFontBold())
	];

	for (const auto& Pair : Component->Inputs)
	{
		const FString& Key = Pair.Key;
		const FPLX_Input& Input = Pair.Value;

		FString InputTypeName = UEnum::GetValueAsString(Input.Type);
		InputTypeName = InputTypeName.RightChop(InputTypeName.Find(TEXT("::")) + 2);

		CategoryBuilder.AddCustomRow(FText::FromString("InputExpandable"))
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
					.Text(FText::FromString(InputTypeName))
					.IsReadOnly(true)
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			]
		];
	}

	// Outputs.
	CategoryBuilder.AddCustomRow(FText::FromString("OutputsHeader"))
	.WholeRowContent()
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString("Outputs"))
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

	// Outputs List
	for (const auto& Pair : Component->Outputs)
	{
		const FString& Key = Pair.Key;
		const FPLX_Output& Output = Pair.Value;

		if (!Component->bShowDisabledOutputs && !Output.bEnabled)
			continue;

		FString OutputTypeName = UEnum::GetValueAsString(Output.Type);
		OutputTypeName = OutputTypeName.RightChop(OutputTypeName.Find(TEXT("::")) + 2);

		CategoryBuilder.AddCustomRow(FText::FromString("OutputExpandable"))
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
					.Text(FText::FromString(OutputTypeName))
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
			]
		];
	}
	// clang-format on
}

#undef LOCTEXT_NAMESPACE
