// Copyright 2025, Algoryx Simulation AB.

#include "Vehicle/AGX_AckermannSteeringParametersTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "Utilities/AGX_SlateUtilities.h"
#include "Vehicle/AGX_AckermannSteeringParameters.h"

#define LOCTEXT_NAMESPACE "FAGX_AckermannSteeringParametersTypeActions"

FAGX_AckermannSteeringParametersTypeActions::FAGX_AckermannSteeringParametersTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_AckermannSteeringParametersTypeActions::GetName() const
{
	return LOCTEXT("AssetName", "AGX Ackermann Steering Parameters");
}

const TArray<FText>& FAGX_AckermannSteeringParametersTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("VehicleSubMenu", "Vehicle"),
	};

	return SubMenus;
}

uint32 FAGX_AckermannSteeringParametersTypeActions::GetCategories()
{
	return AssetCategory;
}

FColor FAGX_AckermannSteeringParametersTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_AckermannSteeringParametersTypeActions::GetAssetDescription(
	const FAssetData& AssetData) const
{
	return LOCTEXT(
		"AssetDescription", "Defines parameters for ackermann steering.");
}

UClass* FAGX_AckermannSteeringParametersTypeActions::GetSupportedClass() const
{
	return UAGX_AckermannSteeringParameters::StaticClass();
}

#undef LOCTEXT_NAMESPACE
