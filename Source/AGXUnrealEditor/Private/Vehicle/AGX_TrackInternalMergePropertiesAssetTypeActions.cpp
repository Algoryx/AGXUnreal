// Copyright 2025, Algoryx Simulation AB.

#include "Vehicle/AGX_TrackInternalMergePropertiesAssetTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "Utilities/AGX_SlateUtilities.h"
#include "Vehicle/AGX_TrackInternalMergeProperties.h"

#define LOCTEXT_NAMESPACE "FAGX_TrackInternalMergePropertiesAssetTypeActions"

FAGX_TrackInternalMergePropertiesAssetTypeActions::
	FAGX_TrackInternalMergePropertiesAssetTypeActions(EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_TrackInternalMergePropertiesAssetTypeActions::GetName() const
{
	return LOCTEXT("AssetName", "AGX Track Internal Merge Properties");
}

const TArray<FText>& FAGX_TrackInternalMergePropertiesAssetTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("TrackSubMenu", "Track"),
	};

	return SubMenus;
}

uint32 FAGX_TrackInternalMergePropertiesAssetTypeActions::GetCategories()
{
	return AssetCategory;
}

FColor FAGX_TrackInternalMergePropertiesAssetTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_TrackInternalMergePropertiesAssetTypeActions::GetAssetDescription(
	const FAssetData& AssetData) const
{
	return LOCTEXT(
		"AssetDescription", "Defines track node merge properties for AGX Track Component.");
}

UClass* FAGX_TrackInternalMergePropertiesAssetTypeActions::GetSupportedClass() const
{
	return UAGX_TrackInternalMergeProperties::StaticClass();
}

#undef LOCTEXT_NAMESPACE
