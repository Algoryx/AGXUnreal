// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class IDetailLayoutBuilder;

/**
 * Deprecated, use AGX_TrackMeshRendererDetails instead.
 */
class AGXUNREALEDITOR_API FAGX_TrackRendererDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder) override;
};
