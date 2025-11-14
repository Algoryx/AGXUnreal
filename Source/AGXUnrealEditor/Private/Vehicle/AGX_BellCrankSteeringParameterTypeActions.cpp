// Copyright 2025, Algoryx Simulation AB.

#include "Vehicle/AGX_BellCrankSteeringParametersTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "Utilities/AGX_SlateUtilities.h"
#include "Vehicle/AGX_BellCrankSteeringParameters.h"

#define LOCTEXT_NAMESPACE "FAGX_BellCrankSteeringParametersTypeActions"

FAGX_BellCrankSteeringParametersTypeActions::FAGX_BellCrankSteeringParametersTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_BellCrankSteeringParametersTypeActions::GetName() const
{
	return LOCTEXT("AssetName", "AGX BellCrank Steering Parameters");
}

const TArray<FText>& FAGX_BellCrankSteeringParametersTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("VehicleSubMenu", "Vehicle"),
	};

	return SubMenus;
}

uint32 FAGX_BellCrankSteeringParametersTypeActions::GetCategories()
{
	return AssetCategory;
}

FColor FAGX_BellCrankSteeringParametersTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_BellCrankSteeringParametersTypeActions::GetAssetDescription(
	const FAssetData& AssetData) const
{
	return LOCTEXT(
		"AssetDescription", "Defines parameters for BellCrank steering.");
}

UClass* FAGX_BellCrankSteeringParametersTypeActions::GetSupportedClass() const
{
	return UAGX_BellCrankSteeringParameters::StaticClass();
}

#undef LOCTEXT_NAMESPACE
