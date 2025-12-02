// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Widgets/SCompoundWidget.h"

class SAGX_GenerateRuntimeActivationDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAGX_GenerateRuntimeActivationDialog)
	{
	}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedRef<SWidget> CreateUserInputGui();

	FText GetLicenseIdText() const;
	void OnLicenseIdTextCommitted(const FText& NewText, ETextCommit::Type InTextCommit);

	FText GetActivationCodeText() const;
	void OnActivationCodeCommitted(const FText& NewText, ETextCommit::Type InTextCommit);

	FReply OnGenerateButtonClicked();

	FText GetReferenceFilePathText() const;
	FReply OnBrowseReferenceFileButtonClicked();

	FText GetLicenseDirPathText() const;
	FReply OnBrowseLicenseDirButtonClicked();

	FString LicenseId;
	FString ActivationCode;
	FString ReferenceFilePath;
	FString LicenseDirPath;
};
