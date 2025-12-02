// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "AssetTypeActions_Base.h"
#include "AssetTypeCategories.h"
#include "CoreMinimal.h"

class AGXUNREALEDITOR_API FAGX_WireMergeSplitThresholdsTypeActions : public FAssetTypeActions_Base
{
public:
	FAGX_WireMergeSplitThresholdsTypeActions(EAssetTypeCategories::Type InAssetCategory);

	virtual FText GetName() const override;

	virtual const TArray<FText>& GetSubMenus() const override;

	virtual uint32 GetCategories() override;

	virtual FColor GetTypeColor() const override;

	virtual FText GetAssetDescription(const FAssetData& AssetData) const override;

	virtual UClass* GetSupportedClass() const override;

private:
	EAssetTypeCategories::Type AssetCategory;
};
