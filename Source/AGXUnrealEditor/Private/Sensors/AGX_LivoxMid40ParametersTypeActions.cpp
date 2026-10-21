// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_LivoxMid40ParametersTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LivoxMid40Parameters.h"
#include "Utilities/AGX_SlateUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_LivoxMid40ParametersTypeActions"

FAGX_LivoxMid40ParametersTypeActions::FAGX_LivoxMid40ParametersTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_LivoxMid40ParametersTypeActions::GetName() const
{
	return LOCTEXT("LivoxMid40ParamsAssetName", "AGX Livox Mid40 Parameters");
}

uint32 FAGX_LivoxMid40ParametersTypeActions::GetCategories()
{
	return AssetCategory;
}

const TArray<FText>& FAGX_LivoxMid40ParametersTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("SensorSubMenu", "Sensor"),
	};

	return SubMenus;
}

FColor FAGX_LivoxMid40ParametersTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_LivoxMid40ParametersTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT("LivoxMid40ParamsDescription", "Holds LivoxMid40 parameter information.");
}

UClass* FAGX_LivoxMid40ParametersTypeActions::GetSupportedClass() const
{
	return UAGX_LivoxMid40Parameters::StaticClass();
}

#undef LOCTEXT_NAMESPACE
