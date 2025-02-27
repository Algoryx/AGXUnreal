// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Import/AGX_ImportEnums.h"

// Unreal Engine includes.
#include "Styling/SlateTypes.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SBorder.h"

class SAGX_ImportDialogBase : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAGX_ImportDialogBase)
	{
	}
	SLATE_END_ARGS()

	virtual void Construct(const FArguments& InArgs) = 0;
	void SetFilePath(const FString& InFilePath);
	void SetIgnoreDisabledTrimeshes(bool bInIgnoreDisabledTrimesh);
	void RefreshGui();

protected:
	TSharedRef<SWidget> CreateBrowseFileGui();
	TSharedRef<SBorder> CreateAGXFileGui();
	TSharedRef<SWidget> CreateIgnoreDisabledTrimeshGui();

	FReply OnBrowseFileButtonClicked();
	FText GetFilePathText() const;
	void OnIgnoreDisabledTrimeshCheckboxClicked(ECheckBoxState NewCheckedState);
	void OnFilePathTextCommitted(const FText& InNewText, ETextCommit::Type InCommitType);

	FString FileTypes;
	EAGX_ImportType ImportType = EAGX_ImportType::Invalid;
	FString FilePath;
	bool bIgnoreDisabledTrimesh = true;
	bool bUserHasPressedImportOrSynchronize = false;
};
