// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "Toolkits/BaseToolkit.h"

/**
 *
 */
class AGXUNREALEDITOR_API FAGX_AgxEdModeToolkit : public FModeToolkit
{
public:
	FAGX_AgxEdModeToolkit();

	// FModeToolkit interface
	virtual void Init(const TSharedPtr<IToolkitHost>& InitToolkitHost) override;
	// End of FModeToolkit interface

	// IToolkit interface
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual class FEdMode* GetEditorMode() const override;
	virtual TSharedPtr<class SWidget> GetInlineContent() const override;
	// End of IToolkit interface

	void OnSubModeChanged();

private:
	TSharedPtr<class SAGX_AgxEdModeWidget> ToolkitWidget;
};
