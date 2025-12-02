// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "Input/Reply.h"

class IDetailLayoutBuilder;
class IDetailCategoryBuilder;

/**
 * Defines the design of the Lidar Ambient Material Asset in the Editor.
 */
class AGXUNREALEDITOR_API FAGX_LidarAmbientMaterialCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder) override;

private:
	FReply OnConfigureAsAirButtonClicked();
	FReply OnConfigureAsFogButtonClicked();
	FReply OnConfigureAsRainfallButtonClicked();
	FReply OnConfigureAsSnowfallButtonClicked();

	IDetailLayoutBuilder* DetailBuilder {nullptr};
	double AirVisibility {3.0};
	double FogVisibility {1.0};
	double FogWavelength {900.0};
	double RainRate {5.0};
	double SnowfallRate {5.0};
	double SnowfallWavelength {900};
};
