// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_RigidBodyComponent.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class IDetailLayoutBuilder;
class IDetailCategoryBuilder;

/**
 * Defines the design of the RigidBody Component in the Editor.
 */
class AGXUNREALEDITOR_API FAGX_RigidBodyComponentCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	//~ Begin IDetailCustomNodeBuilder.
	virtual void CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder) override;
	//~ End IDetailCustomNodeBuilder.

private:
	IDetailLayoutBuilder* DetailBuilder;

	FText ExternalForce;
};
