// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_LidarLambertianOpaqueMaterialTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarLambertianOpaqueMaterial.h"
#include "Utilities/AGX_SlateUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_LidarLambertianOpaqueMaterialTypeActions"

FAGX_LidarLambertianOpaqueMaterialTypeActions::FAGX_LidarLambertianOpaqueMaterialTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_LidarLambertianOpaqueMaterialTypeActions::GetName() const
{
	return LOCTEXT(
		"LidarLambertianOpaqueMaterialAssetName", "AGX Lidar Lambertian Opaque Material");
}

uint32 FAGX_LidarLambertianOpaqueMaterialTypeActions::GetCategories()
{
	return AssetCategory;
}

const TArray<FText>& FAGX_LidarLambertianOpaqueMaterialTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("SensorSubMenu", "Sensor"),
	};

	return SubMenus;
}

FColor FAGX_LidarLambertianOpaqueMaterialTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_LidarLambertianOpaqueMaterialTypeActions::GetAssetDescription(
	const FAssetData& AssetData) const
{
	return LOCTEXT(
		"LidarLambertianOpaqueMaterialAssetDesc",
		"Holds Lidar Lambertian Opaque Material information.");
}

UClass* FAGX_LidarLambertianOpaqueMaterialTypeActions::GetSupportedClass() const
{
	return UAGX_LidarLambertianOpaqueMaterial::StaticClass();
}

#undef LOCTEXT_NAMESPACE
