// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_GenericHorizontalSweepParametersTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_GenericHorizontalSweepParameters.h"
#include "Utilities/AGX_SlateUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_GenericHorizontalSweepParametersTypeActions"

FAGX_GenericHorizontalSweepParametersTypeActions::FAGX_GenericHorizontalSweepParametersTypeActions(EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_GenericHorizontalSweepParametersTypeActions::GetName() const
{
	return LOCTEXT("GenericHorizontalSweepParametersAssetName", "AGX Generic Horizontal Sweep Parameters");
}

uint32 FAGX_GenericHorizontalSweepParametersTypeActions::GetCategories()
{
	return AssetCategory;
}

const TArray<FText>& FAGX_GenericHorizontalSweepParametersTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("SensorSubMenu", "Sensor"),
	};

	return SubMenus;
}

FColor FAGX_GenericHorizontalSweepParametersTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_GenericHorizontalSweepParametersTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT("GenericHorizontalSweepParametersDescription", "Holds Generic Horizontal Sweep Parameters information.");
}

UClass* FAGX_GenericHorizontalSweepParametersTypeActions::GetSupportedClass() const
{
	return UAGX_GenericHorizontalSweepParameters::StaticClass();
}

#undef LOCTEXT_NAMESPACE
