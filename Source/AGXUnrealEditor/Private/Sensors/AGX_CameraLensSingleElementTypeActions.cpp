// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_CameraLensSingleElementTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_CameraLensSingleElement.h"
#include "Utilities/AGX_SlateUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_CameraLensSingleElementTypeActions"

FAGX_CameraLensSingleElementTypeActions::FAGX_CameraLensSingleElementTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_CameraLensSingleElementTypeActions::GetName() const
{
	return LOCTEXT("CameraLensSingleElementAssetName", "AGX Camera Lens Single Element");
}

uint32 FAGX_CameraLensSingleElementTypeActions::GetCategories()
{
	return AssetCategory;
}

const TArray<FText>& FAGX_CameraLensSingleElementTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("SensorSubMenu", "Sensor"),
	};

	return SubMenus;
}

FColor FAGX_CameraLensSingleElementTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_CameraLensSingleElementTypeActions::GetAssetDescription(
	const FAssetData& AssetData) const
{
	return LOCTEXT(
		"CameraLensSingleElementAssetDesc", "Holds Camera single element lens information.");
}

UClass* FAGX_CameraLensSingleElementTypeActions::GetSupportedClass() const
{
	return UAGX_CameraLensSingleElement::StaticClass();
}

#undef LOCTEXT_NAMESPACE
