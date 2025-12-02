// Copyright 2025, Algoryx Simulation AB.

#include "Sensors/AGX_LidarAmbientMaterialTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarAmbientMaterial.h"
#include "Utilities/AGX_SlateUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_LidarAmbientMaterialTypeActions"

FAGX_LidarAmbientMaterialTypeActions::FAGX_LidarAmbientMaterialTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_LidarAmbientMaterialTypeActions::GetName() const
{
	return LOCTEXT(
		"LidarAmbientMaterialAssetName", "AGX Lidar Ambient Material");
}

uint32 FAGX_LidarAmbientMaterialTypeActions::GetCategories()
{
	return AssetCategory;
}

const TArray<FText>& FAGX_LidarAmbientMaterialTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("SensorSubMenu", "Sensor"),
	};

	return SubMenus;
}

FColor FAGX_LidarAmbientMaterialTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_LidarAmbientMaterialTypeActions::GetAssetDescription(
	const FAssetData& AssetData) const
{
	return LOCTEXT(
		"LidarAmbientMaterialAssetDesc",
		"Holds Lidar Ambient Material information.");
}

UClass* FAGX_LidarAmbientMaterialTypeActions::GetSupportedClass() const
{
	return UAGX_LidarAmbientMaterial::StaticClass();
}

#undef LOCTEXT_NAMESPACE
