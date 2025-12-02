// Copyright 2025, Algoryx Simulation AB.

#include "Materials/AGX_ShapeMaterialAssetTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "Materials/AGX_ShapeMaterial.h"
#include "Utilities/AGX_SlateUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_ShapeMaterialTypeActions"

FAGX_ShapeMaterialTypeActions::FAGX_ShapeMaterialTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_ShapeMaterialTypeActions::GetName() const
{
	return LOCTEXT("AssetName", "AGX Shape Material");
}

const TArray<FText>& FAGX_ShapeMaterialTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("ShapeSubMenu", "Shape"),
	};

	return SubMenus;
}

uint32 FAGX_ShapeMaterialTypeActions::GetCategories()
{
	return AssetCategory;
}

FColor FAGX_ShapeMaterialTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_ShapeMaterialTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT("AssetDescription", "Defines bulk and surface properties of AGX Shapes.");
}

UClass* FAGX_ShapeMaterialTypeActions::GetSupportedClass() const
{
	return UAGX_ShapeMaterial::StaticClass();
}

#undef LOCTEXT_NAMESPACE
