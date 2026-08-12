// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainWheelSettingsActions.h"

// AGX Dynamics for Unreal includes.
#include "Terrain/AGX_TerrainWheelSettings.h"
#include "Utilities/AGX_SlateUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_TerrainWheelSettingsActions"

FAGX_TerrainWheelSettingsActions::FAGX_TerrainWheelSettingsActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_TerrainWheelSettingsActions::GetName() const
{
	return LOCTEXT("AssetName", "AGX Terrain Wheel Settings");
}

const TArray<FText>& FAGX_TerrainWheelSettingsActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {LOCTEXT("TerrainSubMenu", "Terrain")};

	return SubMenus;
}

uint32 FAGX_TerrainWheelSettingsActions::GetCategories()
{
	return AssetCategory;
}

FColor FAGX_TerrainWheelSettingsActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_TerrainWheelSettingsActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT("AssetDescription", "Defines settings for AGX Terrain Wheel.");
}

UClass* FAGX_TerrainWheelSettingsActions::GetSupportedClass() const
{
	return UAGX_TerrainWheelSettings::StaticClass();
}

#undef LOCTEXT_NAMESPACE
