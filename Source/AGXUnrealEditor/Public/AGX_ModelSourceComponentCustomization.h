// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "Input/Reply.h"

class UAGX_ModelSourceComponent;

class IDetailLayoutBuilder;
class IDetailCategoryBuilder;
class UMaterialInterface;

/**
 * Defines the design of the Model Source Component in the Editor.
 */
class AGXUNREALEDITOR_API FAGX_ModelSourceComponentCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder) override;

private:
	FReply OnSynchronizeModelButtonClicked();

	// Members related to render Material replacement.
	void CustomizeMaterialReplacer(UAGX_ModelSourceComponent* ModelSource);
	FString GetCurrentMaterialPath() const;
	FString GetNewMaterialPath() const;
	void OnCurrentMaterialSelected(const FAssetData& AssetData);
	void OnNewMaterialSelected(const FAssetData& AssetData);
	bool IncludeOnlyUsedMaterials(const FAssetData& AssetData);
	bool IncludeAllMaterials(const FAssetData& AssetData);
	FReply OnReplaceMaterialsButtonClicked();

	IDetailLayoutBuilder* DetailBuilder;

	// List of Material assets currently in use by the Blueprint.
	TSet<UMaterialInterface*> KnownMaterials;

	friend struct FAGX_ModelSourceComponentCustomization_helper;
};
