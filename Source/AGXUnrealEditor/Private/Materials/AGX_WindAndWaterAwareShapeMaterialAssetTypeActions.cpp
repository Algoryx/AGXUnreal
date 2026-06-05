// Copyright 2023, Algoryx Simulation AB.

#include "Materials/AGX_WindAndWaterAwareShapeMaterialAssetTypeActions.h"

#include "Model/AGX_WindAndWaterAwareShapeMaterial.h"

#define LOCTEXT_NAMESPACE "FAGX_WindAndWaterAwareShapeMaterialTypeActions"

FAGX_WindAndWaterAwareShapeMaterialTypeActions::FAGX_WindAndWaterAwareShapeMaterialTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_WindAndWaterAwareShapeMaterialTypeActions::GetName() const
{
	return LOCTEXT("AssetName", "AGX Wind And Water Aware Shape Material");
}

uint32 FAGX_WindAndWaterAwareShapeMaterialTypeActions::GetCategories()
{
	return AssetCategory;
}

const TArray<FText>& FAGX_WindAndWaterAwareShapeMaterialTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("WindAndWaterSubMenu", "Hydro&Aero"),
	};

	return SubMenus;
}

FColor FAGX_WindAndWaterAwareShapeMaterialTypeActions::GetTypeColor() const
{
	return FColor(255, 115, 0);
}

FText FAGX_WindAndWaterAwareShapeMaterialTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT("AssetDescription", "Defines bulk and surface properties of AGX Shapes.");
}

UClass* FAGX_WindAndWaterAwareShapeMaterialTypeActions::GetSupportedClass() const
{
	return UAGX_WindAndWaterAwareShapeMaterial::StaticClass();
}

#undef LOCTEXT_NAMESPACE
