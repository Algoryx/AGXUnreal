// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_LensDistortionBrownConradyTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LensDistortionBrownConrady.h"
#include "Utilities/AGX_SlateUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_LensDistortionBrownConradyTypeActions"

FAGX_LensDistortionBrownConradyTypeActions::FAGX_LensDistortionBrownConradyTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_LensDistortionBrownConradyTypeActions::GetName() const
{
	return LOCTEXT("LensDistortionBrownConradyAssetName", "AGX Lens Distortion Brown-Conrady");
}

uint32 FAGX_LensDistortionBrownConradyTypeActions::GetCategories()
{
	return AssetCategory;
}

const TArray<FText>& FAGX_LensDistortionBrownConradyTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("SensorSubMenu", "Sensor"),
	};

	return SubMenus;
}

FColor FAGX_LensDistortionBrownConradyTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_LensDistortionBrownConradyTypeActions::GetAssetDescription(
	const FAssetData& AssetData) const
{
	return LOCTEXT(
		"LensDistortionBrownConradyAssetDesc",
		"Holds Brown-Conrady lens distortion information.");
}

UClass* FAGX_LensDistortionBrownConradyTypeActions::GetSupportedClass() const
{
	return UAGX_LensDistortionBrownConrady::StaticClass();
}

#undef LOCTEXT_NAMESPACE
