// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_LidarSensorComponentCustomization.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_CustomRayPatternParameters.h"
#include "Sensors/AGX_GenericHorizontalSweepParameters.h"
#include "Sensors/AGX_LidarSensorComponent.h"
#include "Sensors/AGX_OusterOS0Parameters.h"
#include "Sensors/AGX_OusterOS1Parameters.h"
#include "Sensors/AGX_OusterOS2Parameters.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Utilities/AGX_SensorUtilities.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailLayoutBuilder.h"
#include "Styling/SlateTypes.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FAGX_LidarSensorComponentCustomization"

TSharedRef<IDetailCustomization> FAGX_LidarSensorComponentCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_LidarSensorComponentCustomization);
}

void FAGX_LidarSensorComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	DetailBuilder = &InDetailBuilder;

	UAGX_LidarSensorComponent* Lidar =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_LidarSensorComponent>(
			InDetailBuilder);
	if (Lidar == nullptr)
	{
		return;
	}

	// Hide the default Model widget, we will create a custom one for it.
	InDetailBuilder.HideProperty(
		InDetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAGX_LidarSensorComponent, Model)));

	IDetailCategoryBuilder& CategoryBuilder =
		InDetailBuilder.EditCategory("AGX Lidar", FText::GetEmpty(), ECategoryPriority::Important);

	//clang-format off
	CategoryBuilder.AddCustomRow(FText::GetEmpty())
		.NameContent()[SNew(STextBlock)
						   .Text(LOCTEXT("LidarModel", "Model"))
						   .Font(IDetailLayoutBuilder::GetDetailFont())]
		.ValueContent()
			[SNew(SComboBox<TSharedPtr<EAGX_LidarModel>>)
				 .ContentPadding(2)
				 .OptionsSource(GetAvailableModels())
				 .OnGenerateWidget_Lambda(
					 [=](TSharedPtr<EAGX_LidarModel> Item)
					 {
						 // content for each item in combo box
						 return SNew(STextBlock)
							 .Text(FText::FromString(
								 FAGX_LidarSensorComponentCustomization::FromEnum(*Item)))
							 .ToolTipText(FText::GetEmpty());
					 })
				 .OnSelectionChanged(
					 this, &FAGX_LidarSensorComponentCustomization::OnModelComboBoxChanged)
				 .Content() // header content showing selected item, even while combo box is closed.
					 [SNew(STextBlock)
						  .Text_Lambda(
							  [this]() {
								  return FText::FromString(FAGX_LidarSensorComponentCustomization::
															   GetLidarModelString());
							  })]];

	CategoryBuilder.AddProperty(InDetailBuilder.GetProperty(
		GET_MEMBER_NAME_CHECKED(UAGX_LidarSensorComponent, bEnabled)));

	CategoryBuilder.AddProperty(
		InDetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UAGX_LidarSensorComponent, Range)));

	CategoryBuilder.AddProperty(InDetailBuilder.GetProperty(
		GET_MEMBER_NAME_CHECKED(UAGX_LidarSensorComponent, BeamDivergence)));

	CategoryBuilder.AddProperty(InDetailBuilder.GetProperty(
		GET_MEMBER_NAME_CHECKED(UAGX_LidarSensorComponent, BeamExitRadius)));

	CategoryBuilder.AddProperty(InDetailBuilder.GetProperty(
		GET_MEMBER_NAME_CHECKED(UAGX_LidarSensorComponent, ModelParameters)));

	// Create Lidar Model Parameters Asset Button.
	CategoryBuilder.AddCustomRow(FText::GetEmpty())
		[SNew(SHorizontalBox) +
		 SHorizontalBox::Slot().AutoWidth()
			 [SNew(SButton)
				  .Text(LOCTEXT(
					  "CreateModelParameterAssetsButtonText", "Create Model Parameters Asset"))
				  .ToolTipText(LOCTEXT(
					  "CreateModelParameterAssetsTooltip",
					  "Create a new Model Parameters Asset for the selected Lidar Model."))
				  .OnClicked(
					  this, &FAGX_LidarSensorComponentCustomization::
								OnCreateModelParametersAssetButtonClicked)]];
	//clang-format on

	InDetailBuilder.HideCategory(FName("Sockets"));
}

void FAGX_LidarSensorComponentCustomization::OnModelComboBoxChanged(
	TSharedPtr<EAGX_LidarModel> NewModel, ESelectInfo::Type InSeletionInfo)
{
	UAGX_LidarSensorComponent* Lidar =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_LidarSensorComponent>(
			*DetailBuilder);
	if (Lidar == nullptr || NewModel == nullptr)
		return;

	Lidar->SetModel(*NewModel);

	UClass* NewModelParametersType = FAGX_SensorUtilities::GetParameterTypeFrom(*NewModel);
	if (Lidar->ModelParameters != nullptr && NewModelParametersType != nullptr &&
		!Lidar->ModelParameters->IsA(NewModelParametersType))
	{
		// Clear ModelParameters selection if currently selected does not match the Lidar Model.
		Lidar->ModelParameters = nullptr;
	}

	if (!Lidar->IsCustomParametersSupported())
		Lidar->bEnableDistanceGaussianNoise = false; // Help EditCondition logic for this property.

	for (auto Instance : FAGX_ObjectUtilities::GetArchetypeInstances(*Lidar))
	{
		Instance->CopyFrom(*Lidar);
	}
}

const TArray<TSharedPtr<EAGX_LidarModel>>*
FAGX_LidarSensorComponentCustomization::GetAvailableModels()
{
	if (AvailableModels.Num() == 0)
	{
		AvailableModels.Add(MakeShared<EAGX_LidarModel>(EAGX_LidarModel::CustomRayPattern));
		AvailableModels.Add(MakeShared<EAGX_LidarModel>(EAGX_LidarModel::GenericHorizontalSweep));
		AvailableModels.Add(MakeShared<EAGX_LidarModel>(EAGX_LidarModel::OusterOS0));
		AvailableModels.Add(MakeShared<EAGX_LidarModel>(EAGX_LidarModel::OusterOS1));
		AvailableModels.Add(MakeShared<EAGX_LidarModel>(EAGX_LidarModel::OusterOS2));
	}

	return &AvailableModels;
}

FString FAGX_LidarSensorComponentCustomization::GetLidarModelString() const
{
	UAGX_LidarSensorComponent* Lidar =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_LidarSensorComponent>(
			*DetailBuilder);
	if (Lidar == nullptr)
	{
		return "";
	}

	return FromEnum(Lidar->GetModel());
}

FString FAGX_LidarSensorComponentCustomization::FromEnum(EAGX_LidarModel Model)
{
	switch (Model)
	{
		case EAGX_LidarModel::CustomRayPattern:
			return "CustomRayPattern";
		case EAGX_LidarModel::GenericHorizontalSweep:
			return "GenericHorizontalSweep";
		case EAGX_LidarModel::OusterOS0:
			return "OusterOS0";
		case EAGX_LidarModel::OusterOS1:
			return "OusterOS1";
		case EAGX_LidarModel::OusterOS2:
			return "OusterOS2";
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("Unknown EAGX_LidarModel enum literal passed to "
			 "FAGX_LidarSensorComponentCustomization::FromEnum. Returning 'None'."));
	return "None";
}

namespace AGX_LidarSensorComponentCustomization_helpers
{
	UAGX_LidarModelParameters* CreateModelParametersAsset(
		UClass* Type, const FString& AssetNameSuggestion, const FString& DialogTitle)
	{
		const FString AssetPath =
			FAGX_EditorUtilities::SelectNewAssetDialog(Type, "", AssetNameSuggestion, DialogTitle);

		if (AssetPath.IsEmpty())
			return nullptr;

		const FString AssetName = FPaths::GetBaseFilename(AssetPath);

		UPackage* Package = CreatePackage(*AssetPath);
		auto Asset = NewObject<UAGX_LidarModelParameters>(
			Package, Type, FName(*AssetName), RF_Public | RF_Standalone);
		if (Asset == nullptr)
		{
			FAGX_NotificationUtilities::ShowNotification(
				FString::Printf(TEXT("Unable to create asset given Asset Path: '%s'"), *AssetPath),
				SNotificationItem::ECompletionState::CS_Fail);
			return nullptr;
		}

		return Asset;
	}
}

FReply FAGX_LidarSensorComponentCustomization::OnCreateModelParametersAssetButtonClicked()
{
	using namespace AGX_LidarSensorComponentCustomization_helpers;
	AGX_CHECK(DetailBuilder);

	UAGX_LidarSensorComponent* Lidar =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_LidarSensorComponent>(
			*DetailBuilder);
	if (Lidar == nullptr)
		return FReply::Handled();

	if (Lidar->GetWorld() != nullptr && Lidar->GetWorld()->IsGameWorld())
	{
		FAGX_NotificationUtilities::ShowNotification(
			"This action is not possible during Play.",
			SNotificationItem::ECompletionState::CS_Fail);
		return FReply::Handled();
	}

	// Ask the user to specify file name and location for the Model Parameters Asset to be created.
	const FString DefaultAssetName = FString::Printf(TEXT("LMP_%s"), *FromEnum(Lidar->GetModel()));

	auto ModelParametersType = FAGX_SensorUtilities::GetParameterTypeFrom(Lidar->GetModel());
	if (ModelParametersType == nullptr)
	{
		FAGX_NotificationUtilities::ShowNotification(
			"Unable to create a Model Parameters Asset given the selected Model.",
			SNotificationItem::ECompletionState::CS_Fail);
		return FReply::Handled();
	}

	UAGX_LidarModelParameters* ModelParametersAsset = CreateModelParametersAsset(
		ModelParametersType, DefaultAssetName, FString("Save Model Parameters Asset As"));
	if (ModelParametersAsset == nullptr)
		return FReply::Handled(); // Logging done in CreateModelParametersAsset.

	if (!FAGX_ObjectUtilities::SaveAsset(*ModelParametersAsset))
	{
		FAGX_NotificationUtilities::ShowNotification(
			FString::Printf(TEXT("Unable to save asset: '%s'"), *ModelParametersAsset->GetName()),
			SNotificationItem::ECompletionState::CS_Fail);
		return FReply::Handled();
	}

	// Finally, assign the created Asset.
	for (auto Instance : FAGX_ObjectUtilities::GetArchetypeInstances(*Lidar))
	{
		if (Instance->ModelParameters == Lidar->ModelParameters)
		{
			Instance->ModelParameters = ModelParametersAsset;
		}
	}

	Lidar->ModelParameters = ModelParametersAsset;

	FAGX_NotificationUtilities::ShowDialogBoxWithLogLog(
		FString::Printf(TEXT("Successfully saved: '%s'"), *ModelParametersAsset->GetName()));

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
