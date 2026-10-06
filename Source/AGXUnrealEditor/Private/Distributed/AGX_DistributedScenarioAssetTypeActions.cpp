// Copyright 2026, Algoryx Simulation AB.

#include "Distributed/AGX_DistributedScenarioAssetTypeActions.h"

#include "Distributed/AGX_DistributedScenarioAsset.h"
#include "Utilities/AGX_SlateUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_DistributedScenarioTypeActions"

FAGX_DistributedScenarioTypeActions::FAGX_DistributedScenarioTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_DistributedScenarioTypeActions::GetName() const
{
	return LOCTEXT("AssetName", "AGX Distributed Scenario");
}

const TArray<FText>& FAGX_DistributedScenarioTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {LOCTEXT("DistributedSubMenu", "Distributed")};
	return SubMenus;
}

uint32 FAGX_DistributedScenarioTypeActions::GetCategories()
{
	return AssetCategory;
}

FColor FAGX_DistributedScenarioTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_DistributedScenarioTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT("AssetDescription", "Configures an AGX Distributed client scenario.");
}

UClass* FAGX_DistributedScenarioTypeActions::GetSupportedClass() const
{
	return UAGX_DistributedScenarioAsset::StaticClass();
}

#undef LOCTEXT_NAMESPACE
