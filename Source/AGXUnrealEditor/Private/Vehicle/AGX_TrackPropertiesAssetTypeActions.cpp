// Copyright 2025, Algoryx Simulation AB.

#include "Vehicle/AGX_TrackPropertiesAssetTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "Utilities/AGX_SlateUtilities.h"
#include "Vehicle/AGX_TrackProperties.h"

#define LOCTEXT_NAMESPACE "FAGX_TrackPropertiesAssetTypeActions"

FAGX_TrackPropertiesAssetTypeActions::FAGX_TrackPropertiesAssetTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_TrackPropertiesAssetTypeActions::GetName() const
{
	return LOCTEXT("AssetName", "AGX Track Properties");
}

const TArray<FText>& FAGX_TrackPropertiesAssetTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("TrackSubMenu", "Track"),
	};

	return SubMenus;
}

uint32 FAGX_TrackPropertiesAssetTypeActions::GetCategories()
{
	return AssetCategory;
}

FColor FAGX_TrackPropertiesAssetTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_TrackPropertiesAssetTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT(
		"AssetDescription", "Defines detailed track properties for AGX Track Component.");
}

UClass* FAGX_TrackPropertiesAssetTypeActions::GetSupportedClass() const
{
	return UAGX_TrackProperties::StaticClass();
}

#undef LOCTEXT_NAMESPACE
