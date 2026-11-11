// Copyright 2026, Algoryx Simulation AB.

#pragma once

#include "AssetTypeActions_Base.h"
#include "AssetTypeCategories.h"
#include "CoreMinimal.h"

/** Content Browser actions for AGX Distributed asset descriptors. */
class AGXUNREALEDITOR_API FAGX_DistributedAssetTypeActions : public FAssetTypeActions_Base
{
public:
	explicit FAGX_DistributedAssetTypeActions(EAssetTypeCategories::Type InAssetCategory);

	virtual FText GetName() const override;
	virtual const TArray<FText>& GetSubMenus() const override;
	virtual uint32 GetCategories() override;
	virtual FColor GetTypeColor() const override;
	virtual FText GetAssetDescription(const FAssetData& AssetData) const override;
	virtual UClass* GetSupportedClass() const override;

private:
	EAssetTypeCategories::Type AssetCategory;
};
