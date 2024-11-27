// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "AssetTypeActions_Base.h"
#include "AssetTypeCategories.h"
#include "CoreMinimal.h"

class AGXUNREALEDITOR_API FAGX_OusterOS1ParametersTypeActions : public FAssetTypeActions_Base
{
public:
	FAGX_OusterOS1ParametersTypeActions(EAssetTypeCategories::Type InAssetCategory);

	FText GetName() const override;

	uint32 GetCategories() override;

	virtual const TArray<FText>& GetSubMenus() const override;

	FColor GetTypeColor() const override;

	FText GetAssetDescription(const FAssetData& AssetData) const override;

	UClass* GetSupportedClass() const override;

private:
	EAssetTypeCategories::Type AssetCategory;
};
