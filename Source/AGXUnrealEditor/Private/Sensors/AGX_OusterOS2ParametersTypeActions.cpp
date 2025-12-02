// Copyright 2025, Algoryx Simulation AB.

#include "Sensors/AGX_OusterOS2ParametersTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_OusterOS2Parameters.h"
#include "Utilities/AGX_SlateUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_OusterOS2ParametersTypeActions"

FAGX_OusterOS2ParametersTypeActions::FAGX_OusterOS2ParametersTypeActions(EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_OusterOS2ParametersTypeActions::GetName() const
{
	return LOCTEXT("OusterOS2ParamsAssetName", "AGX Ouster OS2 Parameter");
}

uint32 FAGX_OusterOS2ParametersTypeActions::GetCategories()
{
	return AssetCategory;
}

const TArray<FText>& FAGX_OusterOS2ParametersTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("SensorSubMenu", "Sensor"),
	};

	return SubMenus;
}

FColor FAGX_OusterOS2ParametersTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_OusterOS2ParametersTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT("OusterOS2ParamsDescription", "Holds OusterOS2 Parameter information.");
}

UClass* FAGX_OusterOS2ParametersTypeActions::GetSupportedClass() const
{
	return UAGX_OusterOS2Parameters::StaticClass();
}

#undef LOCTEXT_NAMESPACE
