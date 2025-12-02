// Copyright 2025, Algoryx Simulation AB.

#include "AgxEdMode/AGX_AgxEdModeToolkit.h"

#include "Engine/Selection.h"
#include "EditorModeManager.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

#include "AgxEdMode/AGX_AgxEdMode.h"
#include "AgxEdMode/AGX_AgxEdModeWidget.h"

#define LOCTEXT_NAMESPACE "FAGX_AgxEdModeToolkit"

FAGX_AgxEdModeToolkit::FAGX_AgxEdModeToolkit()
{
}

void FAGX_AgxEdModeToolkit::Init(const TSharedPtr<IToolkitHost>& InitToolkitHost)
{
	ToolkitWidget = SNew(SAGX_AgxEdModeWidget, static_cast<FAGX_AgxEdMode*>(GetEditorMode()));

	FModeToolkit::Init(InitToolkitHost);
}

FName FAGX_AgxEdModeToolkit::GetToolkitFName() const
{
	return FName("AGX_AgxEdModeToolkit");
}

FText FAGX_AgxEdModeToolkit::GetBaseToolkitName() const
{
	return LOCTEXT("BaseToolkitName", "AGX Dynamics Tools");
}

class FEdMode* FAGX_AgxEdModeToolkit::GetEditorMode() const
{
	return GLevelEditorModeTools().GetActiveMode(FAGX_AgxEdMode::EM_AGX_AgxEdModeId);
}

TSharedPtr<class SWidget> FAGX_AgxEdModeToolkit::GetInlineContent() const
{
	return ToolkitWidget;
}

void FAGX_AgxEdModeToolkit::OnSubModeChanged()
{
	if (ToolkitWidget)
	{
		ToolkitWidget->OnSubModeChanged();
	}
}

#undef LOCTEXT_NAMESPACE
