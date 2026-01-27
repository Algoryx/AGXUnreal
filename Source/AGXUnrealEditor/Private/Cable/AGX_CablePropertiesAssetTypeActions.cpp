// Copyright 2025, Algoryx Simulation AB.

#include "Cable/AGX_CablePropertiesAssetTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "Utilities/AGX_SlateUtilities.h"
#include "Cable/AGX_CableProperties.h"

#define LOCTEXT_NAMESPACE "FAGX_CablePropertiesAssetTypeActions"

FAGX_CablePropertiesAssetTypeActions::FAGX_CablePropertiesAssetTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_CablePropertiesAssetTypeActions::GetName() const
{
	return LOCTEXT("AssetName", "AGX Cable Properties");
}

const TArray<FText>& FAGX_CablePropertiesAssetTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("CableSubMenu", "Cable"),
	};

	return SubMenus;
}

uint32 FAGX_CablePropertiesAssetTypeActions::GetCategories()
{
	return AssetCategory;
}

FColor FAGX_CablePropertiesAssetTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_CablePropertiesAssetTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT(
		"AssetDescription", "Defines detailed Cable properties for AGX Cable Component.");
}

UClass* FAGX_CablePropertiesAssetTypeActions::GetSupportedClass() const
{
	return UAGX_CableProperties::StaticClass();
}

#undef LOCTEXT_NAMESPACE
