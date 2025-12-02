// Copyright 2025, Algoryx Simulation AB.

#include "Materials/AGX_TerrainMaterialCustomization.h"

// AGX Dynamics for Unreal includes.
#include "AGX_CustomVersion.h"
#include "Materials/AGX_ShapeMaterial.h"
#include "Materials/AGX_TerrainMaterial.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_ObjectUtilities.h"
#include "Utilities/AGX_SlateUtilities.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FAGX_TerrainMaterialCustomization"

TSharedRef<IDetailCustomization> FAGX_TerrainMaterialCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_TerrainMaterialCustomization);
}

namespace AGX_TerrainMaterialCustomization_helpers
{
	bool IsAssetVersionOlderThanTerrainShapeSplit(UAGX_TerrainMaterial& Tm)
	{
		const auto AssetVersion = Tm.GetLinkerCustomVersion(FAGX_CustomVersion::GUID);
		return AssetVersion < FAGX_CustomVersion::TerrainMaterialShapeMaterialSplit;
	}
}

void FAGX_TerrainMaterialCustomization::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	UAGX_TerrainMaterial* TerrainMaterial =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_TerrainMaterial>(InDetailBuilder);

	if (!TerrainMaterial)
		return;

	DetailBuilder = &InDetailBuilder;
	IDetailCategoryBuilder& CategoryBuilder = InDetailBuilder.EditCategory("Material Properties");

	// This is to help users migrate old Terrain Material Assets before the
	// TerrainMaterialShapeMaterialSplit to the new setup where Terrain and Shape Materials are
	// separated. Remove this once this has been available in a few major releases.
	AddShapeMaterialCreateGui(*TerrainMaterial);
}

void FAGX_TerrainMaterialCustomization::AddShapeMaterialCreateGui(
	UAGX_TerrainMaterial& TerrainMaterial) const
{
	using namespace AGX_TerrainMaterialCustomization_helpers;
	if (DetailBuilder == nullptr)
		return;

	const ECategoryPriority::Type Priority =
		IsAssetVersionOlderThanTerrainShapeSplit(TerrainMaterial) ? ECategoryPriority::Important
																  : ECategoryPriority::Default;

	// Adds helper button to the Details Panel of the Terrain Material to make it easy to save old
	// Shape Material properties into a new Shape Material asset. This is related to the
	// TerrainMaterialShapeMaterialSplit (see internal issue 712).
	IDetailCategoryBuilder& CategoryBuilder =
		DetailBuilder->EditCategory("Terrain Material", FText::GetEmpty(), Priority);

	// clang-format off
	CategoryBuilder.AddCustomRow(FText::GetEmpty())
	[
		SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f))
		.Padding(FMargin(5.0f, 5.0f))
		.Content()
		[
			SNew(SBorder)
			.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f))
			.BorderImage(FAGX_EditorUtilities::GetBrush("ToolPanel.GroupBorder"))
			.Padding(FMargin(5.0f, 5.0f))
			.Content()
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				[
					SNew(STextBlock)
						.Text(this, &FAGX_TerrainMaterialCustomization::GetShapeMaterialPropertiesMigrationText)
						.Font(FAGX_SlateUtilities::CreateFont(10))
						.ColorAndOpacity(FSlateColor(FLinearColor::Red))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(FMargin(0.f, 5.f, 5.f, 5.f))
					[
						SNew(SButton)
						.Text(LOCTEXT("CreateShapeMaterialButtonText", "Create Shape Material"))
						.ToolTipText(LOCTEXT(
							"CreateShapeMaterialTooltip",
							"Takes the Shape Material (surface) properties stored with this Terrain Material "
							"and saves it to a separate Shape Material asset. Note that the created Shape "
							"Material asset must be assigned to a Terrain to take effect. This action is only "
							"necessary for old Terrain Material assets created before the separation of Shape"
							"Material properties from the Terrain Material."))
						.OnClicked(this, &FAGX_TerrainMaterialCustomization::OnCreateShapeMaterialButtonClicked)
					]
				]
			]
		]
	];
	// clang-format on
}

FReply FAGX_TerrainMaterialCustomization::OnCreateShapeMaterialButtonClicked() const
{
	if (DetailBuilder == nullptr)
		return FReply::Handled();

	UAGX_TerrainMaterial* TerrainMaterial =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_TerrainMaterial>(*DetailBuilder);
	if (TerrainMaterial == nullptr)
		return FReply::Handled();

	const FString SmNameSuggestion = [TerrainMaterial]()
	{
		FString Name = TerrainMaterial->GetName();
		if (Name.StartsWith("TM_"))
			Name.RemoveFromStart("TM_");
		else if (Name.StartsWith("AGX_TM_"))
			Name.RemoveFromStart("AGX_TM_");

		Name.InsertAt(0, "AGX_SM_");
		return Name;
	}();

	const FString AssetPath = FAGX_EditorUtilities::SelectNewAssetDialog(
		UAGX_ShapeMaterial::StaticClass(), "", SmNameSuggestion, "Save Shape Material As");

	if (AssetPath.IsEmpty())
		return FReply::Handled();

	const FString AssetName = FPaths::GetBaseFilename(AssetPath);
	UPackage* Package = CreatePackage(*AssetPath);
	auto Asset =
		NewObject<UAGX_ShapeMaterial>(Package, FName(*AssetName), RF_Public | RF_Standalone);

	if (Asset == nullptr)
	{
		FAGX_NotificationUtilities::ShowNotification(
			FString::Printf(TEXT("Unable to create asset given Asset Path: '%s'"), *AssetPath),
			SNotificationItem::ECompletionState::CS_Fail);
		FReply::Handled(); 
	}

	// Copy over the data.
	Asset->Bulk = TerrainMaterial->GetShapeMaterialBulkProperties();
	Asset->Surface = TerrainMaterial->GetShapeMaterialSurfaceProperties();
	Asset->Wire = TerrainMaterial->GetShapeMaterialWireProperties();

	FAGX_ObjectUtilities::SaveAsset(*Asset);

	FAGX_NotificationUtilities::ShowNotification(
		FString::Printf(TEXT("Succesfully created asset: '%s'"), *AssetPath),
		SNotificationItem::ECompletionState::CS_Success);

	return FReply::Handled();
}

FText FAGX_TerrainMaterialCustomization::GetShapeMaterialPropertiesMigrationText() const
{
	using namespace AGX_TerrainMaterialCustomization_helpers;
	if (DetailBuilder == nullptr)
		return FText();

	UAGX_TerrainMaterial* TerrainMaterial =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_TerrainMaterial>(*DetailBuilder);
	if (TerrainMaterial == nullptr)
		return FText();

	if (!IsAssetVersionOlderThanTerrainShapeSplit(*TerrainMaterial))
		return FText();

	return FText::FromString(
		"Important: Shape Material (surface) properties are no longer part\n"
		"of Terrain Materials, instead the Terrain has both a Terrain Material\n"
		"and a Shape Material. This asset was saved before this change was introduced.\n"
		"It is recommended to create a Shape Material from this Terrain Material and\n"
		"assign it to the Terrain that uses this Terrain Material.\n"
		"To remove this warning, simply re-save this asset and re-open it.");
}

#undef LOCTEXT_NAMESPACE
