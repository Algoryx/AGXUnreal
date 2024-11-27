// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_CustomRayPatternParametersTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_CustomRayPatternParameters.h"
#include "Utilities/AGX_SlateUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_CustomRayPatternParametersTypeActions"

FAGX_CustomRayPatternParametersTypeActions::FAGX_CustomRayPatternParametersTypeActions(EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_CustomRayPatternParametersTypeActions::GetName() const
{
	return LOCTEXT("CustomRayPatternParamsAssetName", "AGX Custom Ray Pattern Parameters");
}

uint32 FAGX_CustomRayPatternParametersTypeActions::GetCategories()
{
	return AssetCategory;
}

const TArray<FText>& FAGX_CustomRayPatternParametersTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("SensorSubMenu", "Sensor"),
	};

	return SubMenus;
}

FColor FAGX_CustomRayPatternParametersTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_CustomRayPatternParametersTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT("CustomRayPatternParamsDescription", "Holds Custom Ray Pattern Parameters information.");
}

UClass* FAGX_CustomRayPatternParametersTypeActions::GetSupportedClass() const
{
	return UAGX_CustomRayPatternParameters::StaticClass();
}

#undef LOCTEXT_NAMESPACE
