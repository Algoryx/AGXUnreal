// Copyright 2025, Algoryx Simulation AB.

#pragma once

#if AGXUNREAL_USE_OPENPLX

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class IDetailLayoutBuilder;
/**
 * Defines the design of the Model Source Component in the Editor.
 */
class AGXUNREALEDITOR_API FOpenPLX_SignalHandlerComponentCustomization : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder) override;
};

#endif
