// Copyright 2025, Algoryx Simulation AB.

#include "Materials/AGX_ContactMaterialCustomization.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "Materials/AGX_ContactMaterial.h"
#include "Materials/AGX_ContactMaterialRegistrarComponent.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_SlateUtilities.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FAGX_ContactMaterialCustomization"

TSharedRef<IDetailCustomization> FAGX_ContactMaterialCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_ContactMaterialCustomization);
}

void FAGX_ContactMaterialCustomization::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	DetailBuilder = &InDetailBuilder;

	const UAGX_ContactMaterial* ContactMaterial =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_ContactMaterial>(InDetailBuilder);
	if (ContactMaterial == nullptr)
	{
		return;
	}

	// Determine if this Contact Material is registered in a ContactMaterialRegistrar in the current
	// Level or not.
	// Only done for assets since instances are by definition already in a game world.
	if (ContactMaterial->IsInstance())
		return;

	UWorld* EditorWorld = FAGX_EditorUtilities::GetEditorWorld();
	if (EditorWorld == nullptr)
		return;

	for (const ULevel* Level : EditorWorld->GetLevels())
	{
		for (auto Actor : Level->Actors)
		{
			if (Actor == nullptr)
				continue;

			TArray<UAGX_ContactMaterialRegistrarComponent*> Registrars;
			Actor->GetComponents<UAGX_ContactMaterialRegistrarComponent>(Registrars, false);
			for (auto Registrar : Registrars)
			{
				for (UAGX_ContactMaterial* Cm : Registrar->ContactMaterials)
				{
					if (Cm == ContactMaterial)
						return; // The ContactMaterial is in a ContactMaterialRegistrar.
				}
			}
		}
	}

	// At this point we know the ContactMaterial is NOT in a ContactMaterialRegistrar in the level,
	// and we add a warning text in the Asset editor.
	IDetailCategoryBuilder& CategoryBuilder = InDetailBuilder.EditCategory("Materials");

	// clang-format off
	CategoryBuilder.AddCustomRow(FText::GetEmpty())
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
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(STextBlock)
							.Text(LOCTEXT("WarningText",
								"Note: this Contact Material is not registered in the Level currently open."))
							.Font(FAGX_SlateUtilities::CreateFont(12))
							.ColorAndOpacity(FSlateColor(FLinearColor::Yellow))
					]
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(FMargin(10.0f, 0.0f))
					[
						SNew(SButton)
						.Text(LOCTEXT("RefreshButtonText", "  Refresh  "))
						.ToolTipText(LOCTEXT("RefreshButtonTooltip",
							"Check again if this Contact Material is registered in the level."))
						.OnClicked(this, &FAGX_ContactMaterialCustomization::OnRefreshButtonClicked)
					]
				]
			]
		]
	];
	// clang-format on
}

FReply FAGX_ContactMaterialCustomization::OnRefreshButtonClicked()
{
	AGX_CHECK(DetailBuilder);
	DetailBuilder->ForceRefreshDetails();
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
