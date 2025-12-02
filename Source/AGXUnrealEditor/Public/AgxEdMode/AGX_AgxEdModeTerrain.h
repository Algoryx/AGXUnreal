// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AgxEdMode/AGX_AgxEdModeSubMode.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_AgxEdModeTerrain.generated.h"

/**
 * Sub-mode for AgxEdMode. Used to manage terrain related matters, such as the material library.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", config = EditorPerProjectUserSettings)
class AGXUNREALEDITOR_API UAGX_AgxEdModeTerrain : public UAGX_AgxEdModeSubMode
{
	GENERATED_BODY()

public:
	static UAGX_AgxEdModeTerrain* GetInstance();

public:
	virtual FText GetDisplayName() const override;
	virtual FText GetTooltip() const override;
	virtual FSlateIcon GetIcon() const override;
};
