// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "AssetTypeCategories.h"
#include "AssetTypeActions_Base.h"

/**
 * Asset Type Actions for UAGX_TerrainMaterialAsset, customizing its appearance in the Editor menus
 * and browsers.
 */
class AGXUNREALEDITOR_API FAGX_TerrainMaterialAssetTypeActions : public FAssetTypeActions_Base
{
public:
	FAGX_TerrainMaterialAssetTypeActions(EAssetTypeCategories::Type InAssetCategory);

	virtual FText GetName() const override;

	virtual const TArray<FText>& GetSubMenus() const override;

	virtual uint32 GetCategories() override;

	virtual FColor GetTypeColor() const override;

	virtual FText GetAssetDescription(const FAssetData& AssetData) const override;

	virtual UClass* GetSupportedClass() const override;

private:
	EAssetTypeCategories::Type AssetCategory;
};
