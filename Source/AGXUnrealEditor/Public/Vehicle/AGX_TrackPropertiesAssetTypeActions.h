// Copyright 2022, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeCategories.h"
#include "AssetTypeActions_Base.h"


/**
 * Asset Type Actions for UAGX_TrackProperties, customizing its appearance in the Editor menus
 * and browsers.
 */
class AGXUNREALEDITOR_API FAGX_TrackPropertiesAssetTypeActions : public FAssetTypeActions_Base
{
public:
	FAGX_TrackPropertiesAssetTypeActions(EAssetTypeCategories::Type InAssetCategory);

	FText GetName() const override;

	uint32 GetCategories() override;

	FColor GetTypeColor() const override;

	FText GetAssetDescription(const FAssetData& AssetData) const override;

	UClass* GetSupportedClass() const override;

private:
	EAssetTypeCategories::Type AssetCategory;
};