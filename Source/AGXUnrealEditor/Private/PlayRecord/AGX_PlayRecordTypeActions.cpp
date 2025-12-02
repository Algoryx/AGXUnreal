// Copyright 2025, Algoryx Simulation AB.

#include "PlayRecord/AGX_PlayRecordTypeActions.h"

// AGX Dynamics for Unreal includes.
#include "PlayRecord/AGX_PlayRecord.h"
#include "Utilities/AGX_SlateUtilities.h"

#define LOCTEXT_NAMESPACE "FAGX_PlayRecordTypeActions"

FAGX_PlayRecordTypeActions::FAGX_PlayRecordTypeActions(EAssetTypeCategories::Type InAssetCategory)
	: AssetCategory(InAssetCategory)
{
}

FText FAGX_PlayRecordTypeActions::GetName() const
{
	return LOCTEXT("PlayRecordAssetName", "AGX Play Record");
}

uint32 FAGX_PlayRecordTypeActions::GetCategories()
{
	return AssetCategory;
}

FColor FAGX_PlayRecordTypeActions::GetTypeColor() const
{
	return FAGX_SlateUtilities::GetAGXColorOrange();
}

FText FAGX_PlayRecordTypeActions::GetAssetDescription(const FAssetData& AssetData) const
{
	return LOCTEXT("PlayRecordAssetDescription", "Can hold state information from a Simulation.");
}

UClass* FAGX_PlayRecordTypeActions::GetSupportedClass() const
{
	return UAGX_PlayRecord::StaticClass();
}

#undef LOCTEXT_NAMESPACE
