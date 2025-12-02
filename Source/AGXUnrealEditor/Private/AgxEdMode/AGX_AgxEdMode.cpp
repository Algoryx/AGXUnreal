// Copyright 2025, Algoryx Simulation AB.

#include "AgxEdMode/AGX_AgxEdMode.h"

// AGX Dynamics for Unreal includes.
#include "AgxEdMode/AGX_AgxEdModeConstraints.h"
#include "AgxEdMode/AGX_AgxEdModeFile.h"
#include "AgxEdMode/AGX_AgxEdModeTerrain.h"
#include "AgxEdMode/AGX_AgxEdModeToolkit.h"
#include "Toolkits/ToolkitManager.h"
#include "Utilities/AGX_EditorUtilities.h"

// Unreal Engine includes.
#include "EditorModeManager.h"

#define LOCTEXT_NAMESPACE "FAGX_AgxEdMode"

const FEditorModeID FAGX_AgxEdMode::EM_AGX_AgxEdModeId = TEXT("EM_AGX_AgxEdMode");

FAGX_AgxEdMode::FAGX_AgxEdMode()
{
	SubModes.Add(UAGX_AgxEdModeFile::GetInstance());
	SubModes.Add(UAGX_AgxEdModeConstraints::GetInstance());
	SubModes.Add(UAGX_AgxEdModeTerrain::GetInstance());

	CurrentSubMode = SubModes[0];
}

FAGX_AgxEdMode::~FAGX_AgxEdMode()
{
}

void FAGX_AgxEdMode::Enter()
{
	FEdMode::Enter();

	if (!Toolkit.IsValid() && UsesToolkits())
	{
		Initialize();
		Toolkit = MakeShareable(new FAGX_AgxEdModeToolkit);
		Toolkit->Init(Owner->GetToolkitHost());
	}
}

void FAGX_AgxEdMode::Exit()
{
	if (Toolkit.IsValid())
	{
		FToolkitManager::Get().CloseToolkit(Toolkit.ToSharedRef());
		Toolkit.Reset();
	}

	FEdMode::Exit();
}

bool FAGX_AgxEdMode::UsesToolkits() const
{
	return true;
}

const TArray<UAGX_AgxEdModeSubMode*>& FAGX_AgxEdMode::GetSubModes() const
{
	return SubModes;
}

UAGX_AgxEdModeSubMode* FAGX_AgxEdMode::GetCurrentSubMode() const
{
	return CurrentSubMode;
}

void FAGX_AgxEdMode::SetCurrentSubMode(UAGX_AgxEdModeSubMode* SubMode)
{
	CurrentSubMode = SubMode;

	if (Toolkit)
	{
		StaticCastSharedPtr<FAGX_AgxEdModeToolkit>(Toolkit)->OnSubModeChanged();
	}
}

#undef LOCTEXT_NAMESPACE
