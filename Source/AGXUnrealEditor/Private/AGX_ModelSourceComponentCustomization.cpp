// Copyright 2024, Algoryx Simulation AB.

#include "AGX_ModelSourceComponentCustomization.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_ImporterToBlueprint.h"
#include "AGX_ImportSettings.h"
#include "AGX_ModelSourceComponent.h"
#include "Utilities/AGX_BlueprintUtilities.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_MaterialReplacer.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Widgets/AGX_ImportDialog.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Input/Reply.h"
#include "PropertyCustomizationHelpers.h"
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

	CustomizeMaterialReplacer();

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

struct FAGX_ModelSourceComponentCustomization_helper
{
	using FGetMaterial = FString (FAGX_ModelSourceComponentCustomization::*)() const;
	using FSetMaterial = void (FAGX_ModelSourceComponentCustomization::*)(const FAssetData&);

	static void CreateMaterialWidget(
		FAGX_ModelSourceComponentCustomization& Customizer, IDetailCategoryBuilder& CategoryBuilder,
		const FText& Label, const FText& ToolTip, FGetMaterial GetMaterial,
		FSetMaterial SetMaterial)
	{
		// clang-format off
		CategoryBuilder.AddCustomRow(LOCTEXT("ReplaceMaterial", "Replace Material"))
		.NameContent()
		[
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(Label)
			.ToolTipText(ToolTip)
		]
		.ValueContent()
		[
			SNew(SObjectPropertyEntryBox)
			.AllowedClass(UMaterialInterface::StaticClass())
			.ObjectPath(&Customizer, GetMaterial)
			.OnObjectChanged(&Customizer, SetMaterial)
		];
		// clang-format on
	}
};

void FAGX_ModelSourceComponentCustomization::CustomizeMaterialReplacer()
{
	IDetailCategoryBuilder& CategoryBuilder = DetailBuilder->EditCategory("Material Replacer");

	FAGX_ModelSourceComponentCustomization_helper::CreateMaterialWidget(
		*this, CategoryBuilder, LOCTEXT("CurrentMaterial", "Current Material"),
		LOCTEXT(
			"CurrentMaterialTooltip",
			"The Material currently set on Static Mesh Components that should be replaced with "
			"another Material."),
		&FAGX_ModelSourceComponentCustomization::GetCurrentMaterialPath,
		&FAGX_ModelSourceComponentCustomization::OnCurrentMaterialSelected);
	FAGX_ModelSourceComponentCustomization_helper::CreateMaterialWidget(
		*this, CategoryBuilder, LOCTEXT("NewMaterial", "New Material"),
		LOCTEXT(
			"NewMaterialTooltip",
			"The Material that should be assigned instead of Current Material."),
		&FAGX_ModelSourceComponentCustomization::GetNewMaterialPath,
		&FAGX_ModelSourceComponentCustomization::OnNewMaterialSelected);

	// clang-format off
	CategoryBuilder.AddCustomRow(LOCTEXT("ReplaceMaterialButton", "Replace Material Button"))
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("ReplaceMaterialsButton", "Replace Materials"))
			.ToolTipText(LOCTEXT(
				"ReplaceMaterialsButtonTooltip",
				"Replace all occurrences of Current Material with New Material in this Blueprint"))
			.OnClicked(this, &FAGX_ModelSourceComponentCustomization::OnReplaceMaterialsButtonClicked)
		]
	];
	// clang-format on
}

FString FAGX_ModelSourceComponentCustomization::GetCurrentMaterialPath() const
{
	return FAGX_MaterialReplacer::GetCurrentPathName();
}

FString FAGX_ModelSourceComponentCustomization::GetNewMaterialPath() const
{
	return FAGX_MaterialReplacer::GetNewPathName();
}

void FAGX_ModelSourceComponentCustomization::OnCurrentMaterialSelected(const FAssetData& AssetData)
{
	FAGX_MaterialReplacer::SetCurrent(AssetData);
}

void FAGX_ModelSourceComponentCustomization::OnNewMaterialSelected(const FAssetData& AssetData)
{
	FAGX_MaterialReplacer::SetNew(AssetData);
}

FReply FAGX_ModelSourceComponentCustomization::OnReplaceMaterialsButtonClicked()
{
	auto Bail = [](const TCHAR* Message)
	{
		FAGX_NotificationUtilities::ShowNotification(Message, SNotificationItem::CS_Fail);
		return FReply::Handled();
	};

	if (DetailBuilder == nullptr)
	{
		return Bail(
			TEXT("Material replacing is currenly only supported while there is a Detail Builder."));
	}

	UAGX_ModelSourceComponent* ModelSource =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_ModelSourceComponent>(
			*DetailBuilder);
	if (ModelSource == nullptr)
	{
		return Bail(
			TEXT("Material replacing is currenly only supported with a Model Source Component."));
	}

	UBlueprint* EditBlueprint = FAGX_BlueprintUtilities::GetBlueprintFrom(*ModelSource);
	AActor* Owner = ModelSource->GetOwner();
	if (EditBlueprint == nullptr && Owner == nullptr)
	{
		return Bail(
			TEXT("Material replacing failed because the Model Source Component has neither an "
				 "Owner nor a Blueprint"));
	}

	if (EditBlueprint != nullptr)
	{
		TSharedRef<IPropertyHandle> PropertyHandle = DetailBuilder->GetProperty(
			GET_MEMBER_NAME_CHECKED(UStaticMeshComponent, OverrideMaterials),
			UStaticMeshComponent::StaticClass());
		FAGX_MaterialReplacer::ReplaceMaterials(*EditBlueprint, PropertyHandle.Get());
	}
	else if (Owner != nullptr)
	{
		FAGX_MaterialReplacer::ReplaceMaterials(*Owner);
	}
	else
	{
		checkNoEntry();
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
