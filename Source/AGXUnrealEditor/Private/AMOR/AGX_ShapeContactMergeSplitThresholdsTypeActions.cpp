// Copyright 2025, Algoryx Simulation AB.

#include "AMOR/AGX_ShapeContactMergeSplitThresholdsTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "AMOR/AGX_ShapeContactMergeSplitThresholds.h"
#include "Utilities/AGX_SlateUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_ShapeContactMergeSplitThresholdsTypeActions"

FAGX_ShapeContactMergeSplitThresholdsTypeActions::FAGX_ShapeContactMergeSplitThresholdsTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_ShapeContactMergeSplitThresholdsTypeActions::GetName() const
{
	return LOCTEXT("AssetName", "AGX Shape Contact Merge Split Thresholds");
}

const TArray<FText>& FAGX_ShapeContactMergeSplitThresholdsTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("ShapeSubMenu", "Shape"),
	};

	return SubMenus;
}

uint32 FAGX_ShapeContactMergeSplitThresholdsTypeActions::GetCategories()
{
	return AssetCategory;
}

FColor FAGX_ShapeContactMergeSplitThresholdsTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_ShapeContactMergeSplitThresholdsTypeActions::GetAssetDescription(
	const FAssetData& AssetData) const
{
	return LOCTEXT("AssetDescription", "Defines merge split (AMOR) thresholds for shape contacts.");
}

UClass* FAGX_ShapeContactMergeSplitThresholdsTypeActions::GetSupportedClass() const
{
	return UAGX_ShapeContactMergeSplitThresholds::StaticClass();
}

#undef LOCTEXT_NAMESPACE
