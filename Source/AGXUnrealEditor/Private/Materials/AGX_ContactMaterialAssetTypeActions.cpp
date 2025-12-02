// Copyright 2025, Algoryx Simulation AB.

#include "Materials/AGX_ContactMaterialAssetTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "Materials/AGX_ContactMaterial.h"
#include "Utilities/AGX_SlateUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_ContactMaterialAssetTypeActions"

FAGX_ContactMaterialAssetTypeActions::FAGX_ContactMaterialAssetTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_ContactMaterialAssetTypeActions::GetName() const
{
	return LOCTEXT("AssetName", "AGX Contact Material");
}

const TArray<FText>& FAGX_ContactMaterialAssetTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("ShapeSubMenu", "Shape"),
	};

	return SubMenus;
}

uint32 FAGX_ContactMaterialAssetTypeActions::GetCategories()
{
	return AssetCategory;
}

FColor FAGX_ContactMaterialAssetTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_ContactMaterialAssetTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT(
		"AssetDescription", "Defines detailed material properties for material pair contacts.");
}

UClass* FAGX_ContactMaterialAssetTypeActions::GetSupportedClass() const
{
	return UAGX_ContactMaterial::StaticClass();
}

#undef LOCTEXT_NAMESPACE
