// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_OusterOS1ParametersTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_OusterOS1Parameters.h"
#include "Utilities/AGX_SlateUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_OusterOS1ParametersTypeActions"

FAGX_OusterOS1ParametersTypeActions::FAGX_OusterOS1ParametersTypeActions(EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_OusterOS1ParametersTypeActions::GetName() const
{
	return LOCTEXT("OusterOS1ParamsAssetName", "AGX Ouster OS1 Parameter");
}

uint32 FAGX_OusterOS1ParametersTypeActions::GetCategories()
{
	return AssetCategory;
}

const TArray<FText>& FAGX_OusterOS1ParametersTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("SensorSubMenu", "Sensor"),
	};

	return SubMenus;
}

FColor FAGX_OusterOS1ParametersTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_OusterOS1ParametersTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT("OusterOS1ParamsDescription", "Holds OusterOS1 Parameter information.");
}

UClass* FAGX_OusterOS1ParametersTypeActions::GetSupportedClass() const
{
	return UAGX_OusterOS1Parameters::StaticClass();
}

#undef LOCTEXT_NAMESPACE
