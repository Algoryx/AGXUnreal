// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainWheelMaterialTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "Terrain/AGX_TerrainWheelMaterial.h"
#include "Utilities/AGX_SlateUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_TerrainWheelMaterialTypeActions"

FAGX_TerrainWheelMaterialTypeActions::FAGX_TerrainWheelMaterialTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_TerrainWheelMaterialTypeActions::GetName() const
{
	return LOCTEXT(
		"TerrainWheelMaterialAssetName", "AGX Terrain Wheel Material");
}

uint32 FAGX_TerrainWheelMaterialTypeActions::GetCategories()
{
	return AssetCategory;
}

const TArray<FText>& FAGX_TerrainWheelMaterialTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("SensorSubMenu", "Terrain"),
	};

	return SubMenus;
}

FColor FAGX_TerrainWheelMaterialTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_TerrainWheelMaterialTypeActions::GetAssetDescription(
	const FAssetData& AssetData) const
{
	return LOCTEXT(
		"TerrainWheelMaterialAssetDesc",
		"Holds Terraom Wheel Material information.");
}

UClass* FAGX_TerrainWheelMaterialTypeActions::GetSupportedClass() const
{
	return UAGX_TerrainWheelMaterial::StaticClass();
}

#undef LOCTEXT_NAMESPACE
