// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Widgets/AGX_ImportDialogBase.h"

struct FAGX_ReimportSettings;

class SAGX_SynchronizeModelDialog : public SAGX_ImportDialogBase
{
public:
	void Construct(const FArguments& InArgs) override;

	TOptional<FAGX_ReimportSettings> ToReimportSettings();

private:
	TSharedRef<SBorder> CreateSettingsGui();
	TSharedRef<SBorder> CreateSynchronizeButtonGui();
	TSharedRef<SWidget> CreateForceOverwritePropertiesGui();
	TSharedRef<SWidget> CreateForceReassignRenderMaterialsGui();

	FReply OnSynchronizeButtonClicked();
	void OnForceOverwritePropertiesClicked(ECheckBoxState NewCheckedState);
	void OnForceReassignRenderMaterialsClicked(ECheckBoxState NewCheckedState);

	bool bForceOverwriteProperties = false;
	bool bForceReassignRenderMaterials = false;
};
