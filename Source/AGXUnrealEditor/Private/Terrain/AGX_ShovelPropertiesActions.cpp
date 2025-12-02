// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/AGX_ShovelPropertiesActions.h"

// AGX Dynamics for Unreal includes.
#include "Terrain/AGX_ShovelProperties.h"
#include "Utilities/AGX_SlateUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_ShovelPropertiesActions"

FAGX_ShovelPropertiesActions::FAGX_ShovelPropertiesActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_ShovelPropertiesActions::GetName() const
{
	return LOCTEXT("AssetName", "AGX Shovel Properties");
}

const TArray<FText>& FAGX_ShovelPropertiesActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {LOCTEXT("TerrainSubMenu", "Terrain")};

	return SubMenus;
}

uint32 FAGX_ShovelPropertiesActions::GetCategories()
{
	return AssetCategory;
}

FColor FAGX_ShovelPropertiesActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_ShovelPropertiesActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT(
		"AssetDescription", "Defines detailed Shovel properties for AGX Shovel Component.");
}

UClass* FAGX_ShovelPropertiesActions::GetSupportedClass() const
{
	return UAGX_ShovelProperties::StaticClass();
}

#undef LOCTEXT_NAMESPACE
