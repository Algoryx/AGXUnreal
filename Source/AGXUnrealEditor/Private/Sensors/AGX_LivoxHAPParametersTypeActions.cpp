// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_LivoxHAPParametersTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LivoxHAPParameters.h"
#include "Utilities/AGX_SlateUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_LivoxHAPParametersTypeActions"

FAGX_LivoxHAPParametersTypeActions::FAGX_LivoxHAPParametersTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_LivoxHAPParametersTypeActions::GetName() const
{
	return LOCTEXT("LivoxHAPParamsAssetName", "AGX Livox HAP Parameters");
}

uint32 FAGX_LivoxHAPParametersTypeActions::GetCategories()
{
	return AssetCategory;
}

const TArray<FText>& FAGX_LivoxHAPParametersTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("SensorSubMenu", "Sensor"),
	};

	return SubMenus;
}

FColor FAGX_LivoxHAPParametersTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_LivoxHAPParametersTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT("LivoxHAPParamsDescription", "Holds LivoxHAP parameter information.");
}

UClass* FAGX_LivoxHAPParametersTypeActions::GetSupportedClass() const
{
	return UAGX_LivoxHAPParameters::StaticClass();
}

#undef LOCTEXT_NAMESPACE
