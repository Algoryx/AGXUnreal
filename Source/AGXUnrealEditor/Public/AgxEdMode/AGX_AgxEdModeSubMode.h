// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Textures/SlateIcon.h"

#include "AGX_AgxEdModeSubMode.generated.h"

/**
 * Base class for all sub-modes for AgxEdMode.
 *
 * Each sub-mode gets a toolbar entry in the submode toolbar.
 * The Detail View of the submode object fills the AgxEdMode
 * panel below the toolbar when the toolbar entry is clicked.
 */
UCLASS(abstract, ClassGroup = "AGX", Category = "AGX", config = EditorPerProjectUserSettings)
class AGXUNREALEDITOR_API UAGX_AgxEdModeSubMode : public UObject
{
	GENERATED_BODY()

public:
	virtual FText GetDisplayName() const
		PURE_VIRTUAL(UAGX_AgxEdModeSubMode::GetDisplayName, return FText(););

	virtual FText GetTooltip() const
		PURE_VIRTUAL(UAGX_AgxEdModeSubMode::GetTooltip, return FText(););

	virtual FSlateIcon GetIcon() const
		PURE_VIRTUAL(UAGX_AgxEdModeSubMode::GetIcon, return FSlateIcon(););
};
