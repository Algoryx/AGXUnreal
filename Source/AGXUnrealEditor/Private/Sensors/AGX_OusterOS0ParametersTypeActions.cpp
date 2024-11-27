// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_OusterOS0ParametersTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_OusterOS0Parameters.h"
#include "Utilities/AGX_SlateUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_OusterOS0ParametersTypeActions"

FAGX_OusterOS0ParametersTypeActions::FAGX_OusterOS0ParametersTypeActions(EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_OusterOS0ParametersTypeActions::GetName() const
{
	return LOCTEXT("OusterOS0ParamsAssetName", "AGX Ouster OS0 Parameter");
}

uint32 FAGX_OusterOS0ParametersTypeActions::GetCategories()
{
	return AssetCategory;
}

const TArray<FText>& FAGX_OusterOS0ParametersTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("SensorSubMenu", "Sensor"),
	};

	return SubMenus;
}

FColor FAGX_OusterOS0ParametersTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_OusterOS0ParametersTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT("OusterOS0ParamsDescription", "Holds OusterOS0 Parameter information.");
}

UClass* FAGX_OusterOS0ParametersTypeActions::GetSupportedClass() const
{
	return UAGX_OusterOS0Parameters::StaticClass();
}

#undef LOCTEXT_NAMESPACE
