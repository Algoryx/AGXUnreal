// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "Input/Reply.h"

class IDetailLayoutBuilder;
class IDetailCategoryBuilder;
class UAGX_TerrainMaterial;

/**
 * Defines the design of the Terrain material in the Editor.
 */
class AGXUNREALEDITOR_API FAGX_TerrainMaterialCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder) override;

private:
	void AddShapeMaterialCreateGui(UAGX_TerrainMaterial& TerrainMaterial) const;

	FReply OnCreateShapeMaterialButtonClicked() const;
	FText GetShapeMaterialPropertiesMigrationText() const;

private:
	IDetailLayoutBuilder* DetailBuilder {nullptr};
};
