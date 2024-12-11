// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "Input/Reply.h"

class IDetailLayoutBuilder;
class IDetailCategoryBuilder;

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
	void CustomizeMaterialReplacer();
	FString GetCurrentMaterialPath() const;
	FString GetNewMaterialPath() const;
	void OnCurrentMaterialSelected(const FAssetData& AssetData);
	void OnNewMaterialSelected(const FAssetData& AssetData);
	FReply OnReplaceMaterialsButtonClicked();

	IDetailLayoutBuilder* DetailBuilder;

	friend struct FAGX_ModelSourceComponentCustomization_helper;
};
