#include "Widgets/AGX_LicenseDialog.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Environment.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_StringUtilities.h"

// Unreal Engine includes.
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "SAGX_LicenseDialog"

namespace AGX_LicenseDialog_helpers
{
	FSlateFontInfo CreateFont(int Size)
	{
		FSlateFontInfo F = IPropertyTypeCustomizationUtils::GetRegularFont();
		F.Size = Size;
		return F;
	};

	FString CreateLicenseInfo(const FString& LicenseStatus)
	{
		FString Info("");

		if (!LicenseStatus.IsEmpty())
		{
			Info.Append("Status: " + LicenseStatus + "\n");
		}

		const TArray<FString> Keys {"User", "Contact", "EndDate"};
		for (const auto& Key : Keys)
		{
			if (const auto Value = FAGX_Environment::GetInstance().GetAgxDynamicsLicenseValue(Key))
			{
				Info.Append(Key + ": " + Value.GetValue() + "\n");
			}
		}

		return Info;
	}
}

void SAGX_LicenseDialog::Construct(const FArguments& InArgs)
{
	UpdateLicenseDialogData();

	// clang-format off
	ChildSlot
	[
		SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f))
		.Padding(FMargin(5.0f, 5.0f))
		.Content()
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(FMargin(5.0f, 5.0f))
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f))
				.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
				.Padding(FMargin(5.0f, 5.0f))
				.Content()
				[
					CreateLicenseInfoGui()
				]
			]
			+ SVerticalBox::Slot()
			.Padding(FMargin(5.0f, 5.0f))
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderBackgroundColor(FLinearColor(1.0f, 1.0f, 1.0f))
				.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
				.Padding(FMargin(5.0f, 5.0f))
				.Content()
				[
					CreateLicenseServiceGui()
				]
			]	
		]
	];
	// clang-format on
}

void SAGX_LicenseDialog::UpdateLicenseDialogData()
{
	FString LicenseStatus;
	const bool LicenseValid =
		FAGX_Environment::GetInstance().EnsureAgxDynamicsLicenseValid(&LicenseStatus);

	if (LicenseValid)
	{
		LicenseData.LicenseValidityTextColor = FSlateColor(FLinearColor::Green);
		LicenseData.LicenseValidity = "License: Valid";
	}
	else
	{
		LicenseData.LicenseValidityTextColor = FSlateColor(FLinearColor::Red);
		LicenseData.LicenseValidity = "License: Invalid";
	}

	LicenseData.LicenseInfo = AGX_LicenseDialog_helpers::CreateLicenseInfo(LicenseStatus);

	LicenseData.EnabledModules.Empty();
	for (const FString& Module : FAGX_Environment::GetInstance().GetAgxDynamicsEnabledModules())
	{
		if (Module.Equals("AgX"))
		{
			continue;
		}

		const FString ModuleFormatted = [&Module]()
		{
			FString S = Module;
			return S.Replace(TEXT("AgX-"), TEXT("AGX-"), ESearchCase::CaseSensitive);
		}();

		LicenseData.EnabledModules.Add(MakeShareable(new FString(ModuleFormatted)));
	}

	LicenseData.UsesServiceLicenseWithoutSetupEnv =
		!FAGX_Environment::IsSetupEnvRun() &&
		FAGX_Environment::GetInstance().IsLoadedLicenseOfServiceType();
}

void SAGX_LicenseDialog::RefreshGui()
{
	UpdateLicenseDialogData();
	Construct(FArguments());
}

FText SAGX_LicenseDialog::GetLicenseIdText() const
{
	return FText::FromString(LicenseData.LicenseId);
}

void SAGX_LicenseDialog::OnLicenseIdTextCommitted(
	const FText& NewText, ETextCommit::Type InTextCommit)
{
	LicenseData.LicenseId = NewText.ToString();
}

FText SAGX_LicenseDialog::GetActivationCodeText() const
{
	return FText::FromString(LicenseData.ActivationCode);
}

void SAGX_LicenseDialog::OnActivationCodeCommitted(
	const FText& NewText, ETextCommit::Type InTextCommit)
{
	LicenseData.ActivationCode = NewText.ToString();
}

FReply SAGX_LicenseDialog::OnActivateButtonClicked()
{
	using namespace AGX_LicenseDialog_helpers;
	if (LicenseData.LicenseId.IsEmpty() || LicenseData.ActivationCode.IsEmpty())
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
			"License Id or Activation code was empty. Please enter a License Id and Activation "
			"code.");
		return FReply::Handled();
	}

	if (!ContainsOnlyIntegers(LicenseData.LicenseId))
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
			"License id may only contain integer values.");
		return FReply::Handled();
	}

	const int32 Id = FCString::Atoi(*LicenseData.LicenseId);
	const bool Result = FAGX_Environment::GetInstance().ActivateAgxDynamicsServiceLicense(
		Id, LicenseData.ActivationCode);

	RefreshGui();

	if (!Result)
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
			"License activation was unsuccessful. The Output Log may contain more information.");
		return FReply::Handled();
	}

	FAGX_NotificationUtilities::ShowDialogBoxWithLogLog("License activation was successful.");
	return FReply::Handled();
}

FReply SAGX_LicenseDialog::OnRefreshButtonClicked()
{
	if (!FAGX_Environment::IsAGXDynamicsVersionNewerOrEqualTo(2, 32, 0, 1))
	{
		const FString AGXVersion = FAGX_Environment::GetAGXDynamicsVersion();
		FAGX_NotificationUtilities::ShowDialogBoxWithLogLog(FString::Printf(
			TEXT("Refreshing the service license requires AGX Dynamics version 2.32.0.1 or "
				 "later. The current version is %s. The suggested work-around is to dectivate "
				 "the license and then activating it again."),
			*AGXVersion));
		return FReply::Handled();
	}

	if (!FAGX_Environment::GetInstance().RefreshServiceLicense())
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
			"Refreshing service license was unsuccessful. The Output Log may contain more "
			"information.");
		return FReply::Handled();
	}

	FAGX_NotificationUtilities::ShowDialogBoxWithLogLog("Refreshing service license completed.");
	RefreshGui();
	return FReply::Handled();
}

FReply SAGX_LicenseDialog::OnDeactivateButtonClicked()
{
	const static FString Info =
		"This will deactivate the service license and completely remove "
		"the license file (agx.flx). Are you sure you want to continue?";
	if (FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(Info)) != EAppReturnType::Yes)
	{
		return FReply::Handled();
	}

	if (!FAGX_Environment::GetInstance().DeactivateServiceLicense())
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithErrorLog(
			"License deactivation was unsuccessful. The Output Log may contain more information.");
		return FReply::Handled();
	}

	FAGX_NotificationUtilities::ShowDialogBoxWithLogLog("License deactivation was successful.");
	RefreshGui();
	return FReply::Handled();
}

TSharedRef<SWidget> SAGX_LicenseDialog::CreateLicenseServiceGui()
{
	using namespace AGX_LicenseDialog_helpers;

	// clang-format off
	return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(FMargin(50.0f, 10.0f, 10.f, 10.f))
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("LicenseActivationServiceText", "License activation service"))
				.Font(CreateFont(16))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(5.f, 5.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(FMargin(0.f, 0.f, 45.f, 0.f))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("LicenseIdText", "License id:"))
					.Font(CreateFont(10))
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SEditableTextBox)
					.Text(this, &SAGX_LicenseDialog::GetLicenseIdText)
					.OnTextCommitted(this, &SAGX_LicenseDialog::OnLicenseIdTextCommitted)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(5.f, 5.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(FMargin(0.f, 0.f, 10.f, 0.f))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ActivationCodeText", "Activation code:"))
					.Font(CreateFont(10))
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SEditableTextBox)
					.Text(this, &SAGX_LicenseDialog::GetActivationCodeText)
					.OnTextCommitted(this, &SAGX_LicenseDialog::OnActivationCodeCommitted)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(5.0f, 5.0f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SButton)
					.Text(LOCTEXT("ActivateButtonText", "  Activate  "))
					.ToolTipText(LOCTEXT("ActivateButtonTooltip",
						"Activate AGX Dynamics service license given License id and Activation code."))
					.OnClicked(this, &SAGX_LicenseDialog::OnActivateButtonClicked)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(5.0f, 5.0f))
			[
				CreateRefreshAndDeactivateWidget()
			];
	// clang-format on
}

TSharedRef<SWidget> SAGX_LicenseDialog::CreateLicenseInfoGui()
{
	using namespace AGX_LicenseDialog_helpers;

	// clang-format off
	return SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(FMargin(50.0f, 10.0f, 10.f, 10.f))
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("LicenseInfoText", "License information"))
				.Font(CreateFont(16))
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(5.f, 5.f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					CreateLicenseValidityTextBlock()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(this, &SAGX_LicenseDialog::GetLicenseInfoText)
					.Font(CreateFont(10))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SComboBox<TSharedPtr<FString>>)							
					.OptionsSource(&LicenseData.EnabledModules)
					.OnGenerateWidget_Lambda([=](TSharedPtr<FString> Item)
					{
						return SNew(STextBlock)
							.Text(FText::FromString(*Item));
					})
					.Content()
					[
						SNew(STextBlock)
							.Text(FText::FromString("<Enabled Modules>"))
					]
				]
			];
	// clang-format on
}

TSharedRef<SWidget> SAGX_LicenseDialog::CreateRefreshAndDeactivateWidget()
{
	if (!LicenseData.UsesServiceLicenseWithoutSetupEnv)
	{
		return SNew(SHorizontalBox);
	}

	// clang-format off
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("RefreshButtonText", "  Refresh  "))
			.ToolTipText(LOCTEXT("RefreshButtonTooltip",
				"Refresh AGX Dynamics service license from server."))
			.OnClicked(this, &SAGX_LicenseDialog::OnRefreshButtonClicked)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(FMargin(10.f, 0.f))
		[
			SNew(SButton)
			.Text(LOCTEXT("DeactivateButtonText", "  Deactivate  "))
			.ToolTipText(LOCTEXT("DeactivateButtonTooltip",
				"Deactivate active AGX Dynamics service license and remove the service license "
				"file (agx.lfx) from this project."))
			.OnClicked(this, &SAGX_LicenseDialog::OnDeactivateButtonClicked)
		];
	// clang-format on
}

TSharedRef<SWidget> SAGX_LicenseDialog::CreateLicenseValidityTextBlock() const
{
	using namespace AGX_LicenseDialog_helpers;

	return SNew(STextBlock)
		.Text(this, &SAGX_LicenseDialog::GetLicenseValidityText)
		.Font(CreateFont(10))
		.ColorAndOpacity(this, &SAGX_LicenseDialog::GetLicenseValidityTextColor);
}

FText SAGX_LicenseDialog::GetLicenseValidityText() const
{
	return FText::FromString(LicenseData.LicenseValidity);
}

FSlateColor SAGX_LicenseDialog::GetLicenseValidityTextColor() const
{
	return LicenseData.LicenseValidityTextColor;
}

FText SAGX_LicenseDialog::GetLicenseInfoText() const
{
	return FText::FromString(LicenseData.LicenseInfo);
}

#undef LOCTEXT_NAMESPACE