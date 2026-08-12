// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "AssetTypeActions_Base.h"
#include "AssetTypeCategories.h"
#include "CoreMinimal.h"

/**
 * Asset Type Actions for UAGX_TerrainWheelSettings, customizing its appearance in the Editor menus
 * and browsers.
 */
class AGXUNREALEDITOR_API FAGX_TerrainWheelSettingsActions : public FAssetTypeActions_Base
{
public:
	FAGX_TerrainWheelSettingsActions(EAssetTypeCategories::Type InAssetCategory);

	// ~Begin IAssetTypeActions interface.
	virtual FText GetName() const override;
	virtual const TArray<FText>& GetSubMenus() const override;
	virtual uint32 GetCategories() override;
	virtual FColor GetTypeColor() const override;
	virtual FText GetAssetDescription(const FAssetData& AssetData) const override;
	virtual UClass* GetSupportedClass() const override;
	// ~End IAssetTypeActions interface.

private:
	EAssetTypeCategories::Type AssetCategory;
};
