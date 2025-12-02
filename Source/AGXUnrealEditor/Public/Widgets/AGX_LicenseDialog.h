// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Widgets/SCompoundWidget.h"

class SAGX_LicenseDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SAGX_LicenseDialog)
	{
	}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	void RefreshGui();

	FText GetLicenseIdText() const;
	void OnLicenseIdTextCommitted(const FText& NewText, ETextCommit::Type InTextCommit);

	FText GetActivationCodeText() const;
	void OnActivationCodeCommitted(const FText& NewText, ETextCommit::Type InTextCommit);

	void OnSelectedLicensePathChanged(
		TSharedPtr<FString> SelectedLicensePath, ESelectInfo::Type InSeletionInfo);

	FReply OnActivateButtonClicked();
	FReply OnBrowseExistingLicenseButtonClicked();
	FReply OnCopyExistingLicenseButtonClicked();
	FReply OnRefreshButtonClicked();
	FReply OnDeactivateButtonClicked();

	TSharedRef<SWidget> CreateLicenseServiceGui();
	TSharedRef<SWidget> CreateLicenseInfoGui();
	TSharedRef<SWidget> CreateCopyExistingLicenseGui();
	TSharedRef<SWidget> CreateRefreshAndDeactivateWidget();

	TSharedRef<SWidget> CreateLicenseValidityTextBlock() const;
	FText GetLicenseValidityText() const;
	FSlateColor GetLicenseValidityTextColor() const;

	FText GetLicenseInfoText() const;

	void UpdateLicenseDialogData();

	struct LicenseDialogData
	{
		FString LicenseId;
		FString ActivationCode;
		FString LicenseValidity;
		FString LicenseInfo;
		TArray<TSharedPtr<FString>> ExistingLicenses;
		FString SelectedExistingLicense;
		FSlateColor LicenseValidityTextColor;
		TArray<TSharedPtr<FString>> EnabledModules;
		bool UsesServiceLicenseWithoutSetupEnv = false;
	} LicenseData;
};
