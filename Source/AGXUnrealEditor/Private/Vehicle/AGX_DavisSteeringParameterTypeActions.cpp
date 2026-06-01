// Copyright 2026, Algoryx Simulation AB.

#include "Vehicle/AGX_DavisSteeringParametersTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "Utilities/AGX_SlateUtilities.h"
#include "Vehicle/AGX_DavisSteeringParameters.h"

#define LOCTEXT_NAMESPACE "FAGX_DavisSteeringParametersTypeActions"

FAGX_DavisSteeringParametersTypeActions::FAGX_DavisSteeringParametersTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_DavisSteeringParametersTypeActions::GetName() const
{
	return LOCTEXT("AssetName", "AGX Davis Steering Parameters");
}

const TArray<FText>& FAGX_DavisSteeringParametersTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("VehicleSubMenu", "Vehicle"),
	};

	return SubMenus;
}

uint32 FAGX_DavisSteeringParametersTypeActions::GetCategories()
{
	return AssetCategory;
}

FColor FAGX_DavisSteeringParametersTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_DavisSteeringParametersTypeActions::GetAssetDescription(
	const FAssetData& AssetData) const
{
	return LOCTEXT(
		"AssetDescription", "Defines parameters for Davis steering.");
}

UClass* FAGX_DavisSteeringParametersTypeActions::GetSupportedClass() const
{
	return UAGX_DavisSteeringParameters::StaticClass();
}

#undef LOCTEXT_NAMESPACE
