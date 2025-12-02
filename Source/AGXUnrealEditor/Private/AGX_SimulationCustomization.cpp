// Copyright 2025, Algoryx Simulation AB.

#include "AGX_SimulationCustomization.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Simulation.h"
#include "Sensors/SensorEnvironmentBarrier.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Misc/Paths.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SComboBox.h"

#define LOCTEXT_NAMESPACE "FAGX_SimulationCustomization"

FAGX_SimulationCustomization::FAGX_SimulationCustomization()
{
	const TArray<FString> Devices = FSensorEnvironmentBarrier::GetRaytraceDevices();
	for (int32 I = 0; I < Devices.Num(); I++)
		RaytraceDevices.Add(MakeShared<FString>(FString::Printf(TEXT("[%d] %s"), I, *Devices[I])));
}

TSharedRef<IDetailCustomization> FAGX_SimulationCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_SimulationCustomization);
}

void FAGX_SimulationCustomization::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	DetailBuilder = &InDetailBuilder;

	UAGX_Simulation* Simulation =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_Simulation>(InDetailBuilder);

	if (Simulation == nullptr)
		return;

	// Hide the default ExportPath widget, we will create a custom one for it.
	InDetailBuilder.HideProperty(
		InDetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAGX_Simulation, ExportPath)));

	IDetailCategoryBuilder& CategoryBuilderStartup = InDetailBuilder.EditCategory("Startup");

	CategoryBuilderStartup.AddProperty(
		InDetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAGX_Simulation, bExportInitialState)));

	// clang-format off

	// Create the widgets for browsing to an output file.
	CategoryBuilderStartup.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(FMargin(0.f, 7.f, 25.f, 0.f))
		[
			SNew(STextBlock)
			.Text(LOCTEXT("OutputFilePathText", "Output File:"))
			.Font(IPropertyTypeCustomizationUtils::GetRegularFont())
		]
		+ SHorizontalBox::Slot()
		.Padding(FMargin(0.f, 0.f, 5.f, 0.f))
		.AutoWidth()
		[
			SNew(SEditableTextBox)
			.MinDesiredWidth(150.0f)
			.Text(this, &FAGX_SimulationCustomization::GetOutputFilePathText)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("BrowseButtonText", "Browse..."))
			.ToolTipText(LOCTEXT("BrowseButtonTooltip",
				"Specify an output file for the initial state."))
			.OnClicked(this, &FAGX_SimulationCustomization::OnBrowseFileButtonClicked)
		]
	];
	// clang-format on

	// Fix category ordering.
	InDetailBuilder.EditCategory("Solver");
	InDetailBuilder.EditCategory("Gravity");
	InDetailBuilder.EditCategory("Simulation Stepping Mode");
	InDetailBuilder.EditCategory("Statistics");
	InDetailBuilder.EditCategory("Simulation");
	InDetailBuilder.EditCategory("AGX AMOR");

	// Hide the default RaytraceDeviceIndex widget, we will create a custom one for it.
	InDetailBuilder.HideProperty(
		InDetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAGX_Simulation, RaytraceDeviceIndex)));

	IDetailCategoryBuilder& CategoryBuilderRaytrace = InDetailBuilder.EditCategory("AGX Lidar");

	// clang-format off

	// Create the widgets for selecting Raytrace Device.
	CategoryBuilderRaytrace.AddCustomRow(FText::GetEmpty())
		.NameContent()[SNew(STextBlock)
			.Text(LOCTEXT("RaytraceDevice", "Raytrace Device"))
			.Font(IDetailLayoutBuilder::GetDetailFont())]
		.ValueContent()
		[
			SNew(SComboBox<TSharedPtr<FString>>)
			.OptionsSource(&RaytraceDevices)
			.OnGenerateWidget_Lambda([=](TSharedPtr<FString> Item)
			{
				return SNew(STextBlock)
				.Text(FText::FromString(*Item));
			})
			.OnSelectionChanged(
				this, &FAGX_SimulationCustomization::OnRaytraceDeviceComboBoxChanged)
			.Content()
			[
				SNew(STextBlock)
				.Text_Lambda([this]()
					{ return GetSelectedRaytraceDeviceString(); })
			]
		];
	// clang-format on
}

FText FAGX_SimulationCustomization::GetOutputFilePathText() const
{
	if (DetailBuilder == nullptr)
		return FText();

	UAGX_Simulation* Simulation =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_Simulation>(*DetailBuilder);
	if (Simulation == nullptr)
		return FText();

	return FText::FromString(Simulation->ExportPath);
}

FReply FAGX_SimulationCustomization::OnBrowseFileButtonClicked()
{
	if (DetailBuilder == nullptr)
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithWarning(
			"Unexpected error, unable to get the Detail Builder. Browsing for an output file "
			"will not be possible.");
		return FReply::Handled();
	}

	UAGX_Simulation* Simulation =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_Simulation>(*DetailBuilder);
	if (Simulation == nullptr)
	{
		FAGX_NotificationUtilities::ShowDialogBoxWithWarning(
			"Unexpected error, unable to get the Simulation object. Browsing for an output file "
			"will not be possible.");
		return FReply::Handled();
	}

	const FString StartDir = [Simulation]()
	{
		const FString DirPath = FPaths::GetPath(Simulation->ExportPath);
		return FPaths::DirectoryExists(DirPath) ? DirPath : FString("");
	}();

	FString OutputFilePath = FAGX_EditorUtilities::SelectNewFileDialog(
		"Output file", "AGX Dynamics Archive|*.agx", "AGXUnreal", StartDir);

	if (OutputFilePath.IsEmpty())
		return FReply::Handled();

	Simulation->ExportPath = OutputFilePath;
	return FReply::Handled();
}

void FAGX_SimulationCustomization::OnRaytraceDeviceComboBoxChanged(
	TSharedPtr<FString> SelectedDevice, ESelectInfo::Type InSeletionInfo)
{
	int32 Index {-1};
	RaytraceDevices.Find(SelectedDevice, Index);

	if (Index == INDEX_NONE)
	{
		FAGX_NotificationUtilities::ShowNotification(
			"Unable to map the selected Raytrace Device to a device index. Selection failed.",
			SNotificationItem::CS_Fail);
		return;
	}

	const bool Result = FSensorEnvironmentBarrier::SetCurrentRaytraceDevice(Index);
	if (!Result)
	{
		FAGX_NotificationUtilities::ShowNotification(
			"Unable to set Raytrace Device. The Output Log may contain more information.",
			SNotificationItem::CS_Fail);
		return;
	}

	UAGX_Simulation* Simulation =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_Simulation>(*DetailBuilder);
	if (Simulation == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Unable to get Simulation object in "
				 "FAGX_SimulationCustomization::OnRaytraceDeviceComboBoxChanged."));
		return;
	}

	for (auto Instance : FAGX_ObjectUtilities::GetArchetypeInstances<UAGX_Simulation>(*Simulation))
	{
		if (Instance->RaytraceDeviceIndex == Simulation->RaytraceDeviceIndex)
			Instance->RaytraceDeviceIndex = Index;
	}

	Simulation->RaytraceDeviceIndex = Index;

	// Save to config file.
	const FProperty* Property = UAGX_Simulation::StaticClass()->FindPropertyByName(
		GET_MEMBER_NAME_CHECKED(UAGX_Simulation, RaytraceDeviceIndex));
	Simulation->UpdateSinglePropertyInConfigFile(
		Property, UAGX_Simulation::StaticClass()->GetDefaultConfigFilename());
}

FText FAGX_SimulationCustomization::GetSelectedRaytraceDeviceString()
{
	if (!FSensorEnvironmentBarrier::IsRaytraceSupported())
		return FText::FromString("Raytrace (RTX) not supported");

	UAGX_Simulation* Simulation =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_Simulation>(*DetailBuilder);
	if (Simulation == nullptr)
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Unable to get Simulation object in "
				 "FAGX_SimulationCustomization::GetSelectedRaytraceDeviceString."));
		return FText::FromString("Unknown");
	}

	const int32 CurrentDeviceIndex = Simulation->RaytraceDeviceIndex;
	if (CurrentDeviceIndex < 0)
		return FText::FromString("No Raytrace (RTX) device found");

	if (CurrentDeviceIndex >= RaytraceDevices.Num())
	{
		if (!bHasShownUnableToGetDeviceNameNotification)
		{
			FAGX_NotificationUtilities::ShowNotification(
				"Unable to get name of current Lidar Raytrace (RTX) device",
				SNotificationItem::CS_Fail);
			bHasShownUnableToGetDeviceNameNotification = true;
		}

		return FText::FromString("Unknown");
	}

	return FText::FromString(*RaytraceDevices[CurrentDeviceIndex]);
}

#undef LOCTEXT_NAMESPACE
