// Copyright 2023, Algoryx Simulation AB.

#include "AMOR/AGX_ConstraintMergeSplitThresholdsTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "AMOR/AGX_ConstraintMergeSplitThresholds.h"


#define LOCTEXT_NAMESPACE "FAGX_ConstraintMergeSplitThresholdsTypeActions"

FAGX_ConstraintMergeSplitThresholdsTypeActions::FAGX_ConstraintMergeSplitThresholdsTypeActions(
	EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_ConstraintMergeSplitThresholdsTypeActions::GetName() const
{
	return LOCTEXT("AssetName", "AGX Constraint Merge Split Thresholds");
}

const TArray<FText>& FAGX_ConstraintMergeSplitThresholdsTypeActions::GetSubMenus() const
{
	static const TArray<FText> SubMenus {
		LOCTEXT("ConstraintMergeSplitThSubMenu", "Constraint"),
	};

	return SubMenus;
}

uint32 FAGX_ConstraintMergeSplitThresholdsTypeActions::GetCategories()
{
	return AssetCategory;
}

FColor FAGX_ConstraintMergeSplitThresholdsTypeActions::GetTypeColor() const
{
	return FColor(255, 115, 0);
}

FText FAGX_ConstraintMergeSplitThresholdsTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT(
		"AssetDescription", "Defines merge split (AMOR) thresholds for Constraints.");
}

UClass* FAGX_ConstraintMergeSplitThresholdsTypeActions::GetSupportedClass() const
{
	return UAGX_ConstraintMergeSplitThresholds::StaticClass();
}

#undef LOCTEXT_NAMESPACE