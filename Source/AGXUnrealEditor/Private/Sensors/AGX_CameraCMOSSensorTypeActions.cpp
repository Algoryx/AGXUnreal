// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_CameraCMOSSensorTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_CameraCMOSSensor.h"
#include "Utilities/AGX_SlateUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_CameraCMOSSensorTypeActions"

FAGX_CameraCMOSSensorTypeActions::FAGX_CameraCMOSSensorTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_CameraCMOSSensorTypeActions::GetName() const
{
	return LOCTEXT("CameraCMOSSensorAssetName", "AGX Camera CMOS Sensor");
}

uint32 FAGX_CameraCMOSSensorTypeActions::GetCategories()
{
	return AssetCategory;
}

const TArray<FText>& FAGX_CameraCMOSSensorTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("SensorSubMenu", "Sensor"),
	};

	return SubMenus;
}

FColor FAGX_CameraCMOSSensorTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_CameraCMOSSensorTypeActions::GetAssetDescription(
	const FAssetData& AssetData) const
{
	return LOCTEXT("CameraCMOSSensorAssetDesc", "Holds Camera CMOS Sensor information.");
}

UClass* FAGX_CameraCMOSSensorTypeActions::GetSupportedClass() const
{
	return UAGX_CameraCMOSSensor::StaticClass();
}

#undef LOCTEXT_NAMESPACE
