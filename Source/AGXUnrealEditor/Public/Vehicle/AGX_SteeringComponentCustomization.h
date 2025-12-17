// Copyright 2025, Algoryx Simulation AB.

#pragma once


// Unreal Engine includes.
#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class IDetailLayoutBuilder;
class IDetailCategoryBuilder;

/**
 * Defines the design of the Steering Component in the Editor.
 */
class AGXUNREALEDITOR_API FAGX_SteeringComponentCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	//~ Begin IDetailCustomNodeBuilder.
	virtual void CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder) override;
	//~ End IDetailCustomNodeBuilder.
};
