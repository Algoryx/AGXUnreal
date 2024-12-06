// Copyright 2024, Algoryx Simulation AB.

#include "AGX_ModelSourceComponentCustomization.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_ImporterToBlueprint.h"
#include "AGX_ImportSettings.h"
#include "Import/AGX_ModelSourceComponent.h"
#include "Utilities/AGX_BlueprintUtilities.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Widgets/AGX_ImportDialog.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Input/Reply.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "FAGX_ModelSourceComponentCustomization"

TSharedRef<IDetailCustomization> FAGX_ModelSourceComponentCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_ModelSourceComponentCustomization);
}

void FAGX_ModelSourceComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	DetailBuilder = &InDetailBuilder;

	UAGX_ModelSourceComponent* ModelSourceComponent =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_ModelSourceComponent>(
			InDetailBuilder);
	if (!ModelSourceComponent)
	{
		return;
	}

	IDetailCategoryBuilder& CategoryBuilder = InDetailBuilder.EditCategory("AGX Synchronize Model");

	// clang-format off
	CategoryBuilder.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("SynchronizeModelButtonText", "Synchronize Model"))
			.ToolTipText(LOCTEXT(
				"SynchronizeModelTooltip",
				"Synchronizes the model against the source file of the original "
				"import and updates the Components and Assets to match said source file."))
			.OnClicked(this, &FAGX_ModelSourceComponentCustomization::OnSynchronizeModelButtonClicked)
		]
	];
	// clang-format on

	InDetailBuilder.HideCategory(FName("AGX Synchronize Model Info"));
	InDetailBuilder.HideCategory(FName("Variable"));
	InDetailBuilder.HideCategory(FName("Sockets"));
	InDetailBuilder.HideCategory(FName("Tags"));
	InDetailBuilder.HideCategory(FName("ComponentTick"));
	InDetailBuilder.HideCategory(FName("ComponentReplication"));
	InDetailBuilder.HideCategory(FName("Activation"));
	InDetailBuilder.HideCategory(FName("Cooking"));
	InDetailBuilder.HideCategory(FName("Events"));
	InDetailBuilder.HideCategory(FName("AssetUserData"));
	InDetailBuilder.HideCategory(FName("Collision"));
	InDetailBuilder.HideCategory(FName("Replication"));
}

namespace AGX_ModelSourceComponentCustomization_helpers
{
	UBlueprint* GetBlueprint(IDetailLayoutBuilder& DetailBuilder)
	{
		UAGX_ModelSourceComponent* ModelSourceComponent =
			FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_ModelSourceComponent>(
				DetailBuilder);
		if (ModelSourceComponent == nullptr)
		{
			return nullptr;
		}

		if (!ModelSourceComponent->IsInBlueprint())
		{
			FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
				"Model synchronization is only supported when in a Blueprint.");
			return nullptr;
		}

		if (auto Bp = Cast<UBlueprintGeneratedClass>(ModelSourceComponent->GetOuter()))
		{
			if (Bp->SimpleConstructionScript != nullptr)
			{
				return Bp->SimpleConstructionScript->GetBlueprint();
			}
		}

		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
			"Unable to get the Blueprint from the AGX Model Source Component. Model "
			"synchronization will not be possible.");
		return nullptr;
	}
}

FReply FAGX_ModelSourceComponentCustomization::OnSynchronizeModelButtonClicked()
{
	AGX_CHECK(DetailBuilder);
	using namespace AGX_ModelSourceComponentCustomization_helpers;
	UBlueprint* Blueprint = GetBlueprint(*DetailBuilder);
	if (Blueprint == nullptr)
	{
		// Logging done in GetBlueprint.
		return FReply::Handled();
	}

	FAGX_EditorUtilities::SynchronizeModel(*Blueprint);

	// Any logging is done in SynchronizeModel.
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
