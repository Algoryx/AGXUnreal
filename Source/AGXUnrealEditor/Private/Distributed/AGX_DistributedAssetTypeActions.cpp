// Copyright 2026, Algoryx Simulation AB.

#include "Distributed/AGX_DistributedAssetTypeActions.h"

#include "Distributed/AGX_DistributedAsset.h"
#include "Utilities/AGX_SlateUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_DistributedAssetTypeActions"

FAGX_DistributedAssetTypeActions::FAGX_DistributedAssetTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_DistributedAssetTypeActions::GetName() const
{
	return LOCTEXT("AssetName", "AGX Distributed Asset");
}

const TArray<FText>& FAGX_DistributedAssetTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {LOCTEXT("DistributedSubMenu", "Distributed")};
	return SubMenus;
}

uint32 FAGX_DistributedAssetTypeActions::GetCategories()
{
	return AssetCategory;
}

FColor FAGX_DistributedAssetTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_DistributedAssetTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT(
		"AssetDescription", "Maps an Unreal Actor class to a distributed asset library entry.");
}

UClass* FAGX_DistributedAssetTypeActions::GetSupportedClass() const
{
	return UAGX_DistributedAsset::StaticClass();
}

#undef LOCTEXT_NAMESPACE
