// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_LivoxMid70ParametersTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LivoxMid70Parameters.h"
#include "Utilities/AGX_SlateUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_LivoxMid70ParametersTypeActions"

FAGX_LivoxMid70ParametersTypeActions::FAGX_LivoxMid70ParametersTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_LivoxMid70ParametersTypeActions::GetName() const
{
	return LOCTEXT("LivoxMid70ParamsAssetName", "AGX Livox Mid70 Parameters");
}

uint32 FAGX_LivoxMid70ParametersTypeActions::GetCategories()
{
	return AssetCategory;
}

const TArray<FText>& FAGX_LivoxMid70ParametersTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("SensorSubMenu", "Sensor"),
	};

	return SubMenus;
}

FColor FAGX_LivoxMid70ParametersTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_LivoxMid70ParametersTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT("LivoxMid70ParamsDescription", "Holds LivoxMid70 parameter information.");
}

UClass* FAGX_LivoxMid70ParametersTypeActions::GetSupportedClass() const
{
	return UAGX_LivoxMid70Parameters::StaticClass();
}

#undef LOCTEXT_NAMESPACE
