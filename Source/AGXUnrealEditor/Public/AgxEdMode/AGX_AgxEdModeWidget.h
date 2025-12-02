// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

class IDetailsView;
class FAGX_AgxEdMode;

/**
 *
 */
class AGXUNREALEDITOR_API SAGX_AgxEdModeWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAGX_AgxEdModeWidget)
	{
	}
	SLATE_END_ARGS()

	// Slate function
	void Construct(const FArguments& InArgs, FAGX_AgxEdMode* InAgxEdMode);

	/** Invoke whenever Current SubMode has changed, to update Details View content. */
	void OnSubModeChanged();

private:
	TSharedRef<SWidget> CreateSubModesToolbar();
	TSharedRef<IDetailsView> CreateSubModeDetailsView();

private:
	FAGX_AgxEdMode* AgxEdMode = nullptr;
	TSharedPtr<IDetailsView> SubModeDetailsView = nullptr;
};
