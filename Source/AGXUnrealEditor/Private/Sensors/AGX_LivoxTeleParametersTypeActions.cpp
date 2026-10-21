// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_LivoxTeleParametersTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LivoxTeleParameters.h"
#include "Utilities/AGX_SlateUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_LivoxTeleParametersTypeActions"

FAGX_LivoxTeleParametersTypeActions::FAGX_LivoxTeleParametersTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_LivoxTeleParametersTypeActions::GetName() const
{
	return LOCTEXT("LivoxTeleParamsAssetName", "AGX Livox Tele Parameters");
}

uint32 FAGX_LivoxTeleParametersTypeActions::GetCategories()
{
	return AssetCategory;
}

const TArray<FText>& FAGX_LivoxTeleParametersTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("SensorSubMenu", "Sensor"),
	};

	return SubMenus;
}

FColor FAGX_LivoxTeleParametersTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_LivoxTeleParametersTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT("LivoxTeleParamsDescription", "Holds LivoxTele parameter information.");
}

UClass* FAGX_LivoxTeleParametersTypeActions::GetSupportedClass() const
{
	return UAGX_LivoxTeleParameters::StaticClass();
}

#undef LOCTEXT_NAMESPACE
