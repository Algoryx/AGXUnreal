// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Widgets/SCompoundWidget.h"

class SAGX_OfflineActivationDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAGX_OfflineActivationDialog)
	{
	}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	TSharedRef<SWidget> CreateActivationRequestGui();
	TSharedRef<SWidget> CreateActivationResponseGui();

	FText GetLicenseIdText() const;
	void OnLicenseIdTextCommitted(const FText& NewText, ETextCommit::Type InTextCommit);

	FText GetActivationCodeText() const;
	void OnActivationCodeCommitted(const FText& NewText, ETextCommit::Type InTextCommit);

	FReply OnGenerateActivationRequestButtonClicked();

	FReply OnBrowseResponseFileButtonClicked();
	FText GetActivationResponsePathText() const;

	FReply OnGenerateLicenseButtonClicked();

	FString LicenseId;
	FString ActivationCode;
	FString ActivationResponsePath;
};
