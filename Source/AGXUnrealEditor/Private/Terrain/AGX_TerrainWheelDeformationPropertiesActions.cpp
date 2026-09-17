// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainWheelDeformationPropertiesActions.h"

// AGX Dynamics for Unreal includes.
#include "Terrain/AGX_TerrainWheelDeformationProperties.h"
#include "Utilities/AGX_SlateUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_TerrainWheelDeformationPropertiesActions"

FAGX_TerrainWheelDeformationPropertiesActions::FAGX_TerrainWheelDeformationPropertiesActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_TerrainWheelDeformationPropertiesActions::GetName() const
{
	return LOCTEXT("AssetName", "AGX Terrain Wheel Deformation Properties");
}

const TArray<FText>& FAGX_TerrainWheelDeformationPropertiesActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {LOCTEXT("TerrainSubMenu", "Terrain")};

	return SubMenus;
}

uint32 FAGX_TerrainWheelDeformationPropertiesActions::GetCategories()
{
	return AssetCategory;
}

FColor FAGX_TerrainWheelDeformationPropertiesActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_TerrainWheelDeformationPropertiesActions::GetAssetDescription(
	const FAssetData& AssetData) const
{
	return LOCTEXT(
		"AssetDescription", "Defines deformation properties for AGX Terrain Wheel.");
}

UClass* FAGX_TerrainWheelDeformationPropertiesActions::GetSupportedClass() const
{
	return UAGX_TerrainWheelDeformationProperties::StaticClass();
}

#undef LOCTEXT_NAMESPACE
