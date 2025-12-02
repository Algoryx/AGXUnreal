// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "IDetailCustomization.h"

class FAGX_AgxEdModeTerrainCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

public:
	// ~Begin IDetailCustomization interface.
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	// ~End IDetailCustomization interface.
};
