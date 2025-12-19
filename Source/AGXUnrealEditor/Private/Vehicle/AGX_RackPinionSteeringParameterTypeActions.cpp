// Copyright 2025, Algoryx Simulation AB.

#include "Vehicle/AGX_RackPinionSteeringParametersTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "Utilities/AGX_SlateUtilities.h"
#include "Vehicle/AGX_RackPinionSteeringParameters.h"

#define LOCTEXT_NAMESPACE "FAGX_RackPinionSteeringParametersTypeActions"

FAGX_RackPinionSteeringParametersTypeActions::FAGX_RackPinionSteeringParametersTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_RackPinionSteeringParametersTypeActions::GetName() const
{
	return LOCTEXT("AssetName", "AGX RackPinion Steering Parameters");
}

const TArray<FText>& FAGX_RackPinionSteeringParametersTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("VehicleSubMenu", "Vehicle"),
	};

	return SubMenus;
}

uint32 FAGX_RackPinionSteeringParametersTypeActions::GetCategories()
{
	return AssetCategory;
}

FColor FAGX_RackPinionSteeringParametersTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_RackPinionSteeringParametersTypeActions::GetAssetDescription(
	const FAssetData& AssetData) const
{
	return LOCTEXT(
		"AssetDescription", "Defines parameters for RackPinion steering.");
}

UClass* FAGX_RackPinionSteeringParametersTypeActions::GetSupportedClass() const
{
	return UAGX_RackPinionSteeringParameters::StaticClass();
}

#undef LOCTEXT_NAMESPACE
