// Copyright 2026, Algoryx Simulation AB.

#pragma once

#include "AssetTypeActions_Base.h"
#include "AssetTypeCategories.h"
#include "CoreMinimal.h"

/** Content Browser actions for AGX Distributed scenario assets. */
class AGXUNREALEDITOR_API FAGX_DistributedScenarioTypeActions : public FAssetTypeActions_Base
{
public:
	explicit FAGX_DistributedScenarioTypeActions(EAssetTypeCategories::Type InAssetCategory);

	virtual FText GetName() const override;
	virtual const TArray<FText>& GetSubMenus() const override;
	virtual uint32 GetCategories() override;
	virtual FColor GetTypeColor() const override;
	virtual FText GetAssetDescription(const FAssetData& AssetData) const override;
	virtual UClass* GetSupportedClass() const override;

private:
	EAssetTypeCategories::Type AssetCategory;
};
