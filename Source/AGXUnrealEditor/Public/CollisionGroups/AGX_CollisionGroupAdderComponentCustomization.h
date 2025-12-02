// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class UAGX_CollisionGroupAdderComponent;

class IDetailLayoutBuilder;
class IDetailCategoryBuilder;

/**
 * Defines the design of the Collision Group component in the Editor.
 */
class AGXUNREALEDITOR_API FAGX_CollisionGroupAdderComponentCustomization
	: public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
