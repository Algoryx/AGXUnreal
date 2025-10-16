// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "Types/SlateEnums.h"

class FReply;
class IDetailLayoutBuilder;
class IDetailCategoryBuilder;

/**
 * Defines the design of the IMU Sensor Component in the Editor's details panel.
 */
class AGXUNREALEDITOR_API FAGX_IMUSensorComponentCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
};
