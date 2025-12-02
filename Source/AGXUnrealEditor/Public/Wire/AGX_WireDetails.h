// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Wire/AGX_WireComponent.h"

// Unreal Engine includes.
#include "IDetailCustomization.h"
#include "UObject/WeakObjectPtrTemplates.h"

class FAGX_WireDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	//~ Begin IDetailCustomization interface.
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	//~ End IDetailCustomization interface.

public:
	TWeakObjectPtr<UAGX_WireComponent> Wire;
};
