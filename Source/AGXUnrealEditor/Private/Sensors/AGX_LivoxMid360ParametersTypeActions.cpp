// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_LivoxMid360ParametersTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LivoxMid360Parameters.h"
#include "Utilities/AGX_SlateUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_LivoxMid360ParametersTypeActions"

FAGX_LivoxMid360ParametersTypeActions::FAGX_LivoxMid360ParametersTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_LivoxMid360ParametersTypeActions::GetName() const
{
	return LOCTEXT("LivoxMid360ParamsAssetName", "AGX Livox Mid360 Parameters");
}

uint32 FAGX_LivoxMid360ParametersTypeActions::GetCategories()
{
	return AssetCategory;
}

const TArray<FText>& FAGX_LivoxMid360ParametersTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("SensorSubMenu", "Sensor"),
	};

	return SubMenus;
}

FColor FAGX_LivoxMid360ParametersTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_LivoxMid360ParametersTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT("LivoxMid360ParamsDescription", "Holds LivoxMid360 parameter information.");
}

UClass* FAGX_LivoxMid360ParametersTypeActions::GetSupportedClass() const
{
	return UAGX_LivoxMid360Parameters::StaticClass();
}

#undef LOCTEXT_NAMESPACE
