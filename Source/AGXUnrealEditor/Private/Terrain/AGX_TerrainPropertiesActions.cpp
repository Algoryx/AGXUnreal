// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainPropertiesActions.h"

// AGX Dynamics for Unreal includes.
#include "Terrain/AGX_TerrainProperties.h"
#include "Utilities/AGX_SlateUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_TerrainPropertiesActions"

FAGX_TerrainPropertiesActions::FAGX_TerrainPropertiesActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_TerrainPropertiesActions::GetName() const
{
	return LOCTEXT("AssetName", "AGX Terrain Properties");
}

const TArray<FText>& FAGX_TerrainPropertiesActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {LOCTEXT("TerrainSubMenu", "Terrain")};

	return SubMenus;
}

uint32 FAGX_TerrainPropertiesActions::GetCategories()
{
	return AssetCategory;
}

FColor FAGX_TerrainPropertiesActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_TerrainPropertiesActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT(
		"AssetDescription", "Defines detailed Terrain properties for AGX Terrain.");
}

UClass* FAGX_TerrainPropertiesActions::GetSupportedClass() const
{
	return UAGX_TerrainProperties::StaticClass();
}

#undef LOCTEXT_NAMESPACE
