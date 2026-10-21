// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_LivoxHorizonParametersTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LivoxHorizonParameters.h"
#include "Utilities/AGX_SlateUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_LivoxHorizonParametersTypeActions"

FAGX_LivoxHorizonParametersTypeActions::FAGX_LivoxHorizonParametersTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_LivoxHorizonParametersTypeActions::GetName() const
{
	return LOCTEXT("LivoxHorizonParamsAssetName", "AGX Livox Horizon Parameters");
}

uint32 FAGX_LivoxHorizonParametersTypeActions::GetCategories()
{
	return AssetCategory;
}

const TArray<FText>& FAGX_LivoxHorizonParametersTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("SensorSubMenu", "Sensor"),
	};

	return SubMenus;
}

FColor FAGX_LivoxHorizonParametersTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_LivoxHorizonParametersTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT("LivoxHorizonParamsDescription", "Holds LivoxHorizon parameter information.");
}

UClass* FAGX_LivoxHorizonParametersTypeActions::GetSupportedClass() const
{
	return UAGX_LivoxHorizonParameters::StaticClass();
}

#undef LOCTEXT_NAMESPACE
