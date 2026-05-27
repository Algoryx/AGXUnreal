// Copyright 2023, Algoryx Simulation AB.

#pragma once


// Unreal Engine includes.
#include "AssetTypeActions_Base.h"
#include "AssetTypeCategories.h"
#include "CoreMinimal.h"

/**
 * Asset Type Actions for UAGX_WindAndWaterAwareShapeMaterial, customizing its appearance in the Editor menus
 * and browsers.
 */
class AGXUNREALEDITOR_API FAGX_WindAndWaterAwareShapeMaterialTypeActions : public FAssetTypeActions_Base
{
public:
	FAGX_WindAndWaterAwareShapeMaterialTypeActions(EAssetTypeCategories::Type InAssetCategory);

	FText GetName() const override;

	uint32 GetCategories() override;
	const TArray<FText>& GetSubMenus() const;

	FColor GetTypeColor() const override;

	FText GetAssetDescription(const FAssetData& AssetData) const override;

	UClass* GetSupportedClass() const override;

private:
	EAssetTypeCategories::Type AssetCategory;
};
