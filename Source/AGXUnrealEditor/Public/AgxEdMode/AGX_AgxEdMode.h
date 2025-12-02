// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "EdMode.h"

class UAGX_AgxEdModeSubMode;

/**
 * Top class for the AGX Editor Mode in the Modes panel (to the left of the Editor).
 *
 * The AGX Editor Mode containts a number of Sub Modes, a toolbar to switch between them,
 * and a Detail View for the currently selected Sub Mode. Sub modes classes inherit
 * from UAGX_AgxEdModeSubMode.
 *
 * By Unreal standards, it is actually the Toolkit (FAGX_AgxEdModeToolkit) member of
 * the base class FEdMode that creates the user interface (SAGX_AgxEdModeWidget).
 *
 */
class AGXUNREALEDITOR_API FAGX_AgxEdMode : public FEdMode
{
public:
	const static FEditorModeID EM_AGX_AgxEdModeId;

public:
	FAGX_AgxEdMode();
	virtual ~FAGX_AgxEdMode();

	// FEdMode interface
	virtual void Enter() override;
	virtual void Exit() override;
	bool UsesToolkits() const override;
	// End of FEdMode interface

	const TArray<UAGX_AgxEdModeSubMode*>& GetSubModes() const;

	UAGX_AgxEdModeSubMode* GetCurrentSubMode() const;

	void SetCurrentSubMode(UAGX_AgxEdModeSubMode* SubMode);

private:
	TArray<UAGX_AgxEdModeSubMode*> SubModes;

	UAGX_AgxEdModeSubMode* CurrentSubMode = nullptr;
};
