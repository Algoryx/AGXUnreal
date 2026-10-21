// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_LivoxAviaParametersTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LivoxAviaParameters.h"
#include "Utilities/AGX_SlateUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_LivoxAviaParametersTypeActions"

FAGX_LivoxAviaParametersTypeActions::FAGX_LivoxAviaParametersTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_LivoxAviaParametersTypeActions::GetName() const
{
	return LOCTEXT("LivoxAviaParamsAssetName", "AGX Livox Avia Parameters");
}

uint32 FAGX_LivoxAviaParametersTypeActions::GetCategories()
{
	return AssetCategory;
}

const TArray<FText>& FAGX_LivoxAviaParametersTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("SensorSubMenu", "Sensor"),
	};

	return SubMenus;
}

FColor FAGX_LivoxAviaParametersTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_LivoxAviaParametersTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT("LivoxAviaParamsDescription", "Holds LivoxAvia parameter information.");
}

UClass* FAGX_LivoxAviaParametersTypeActions::GetSupportedClass() const
{
	return UAGX_LivoxAviaParameters::StaticClass();
}

#undef LOCTEXT_NAMESPACE
