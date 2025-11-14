// Copyright 2025, Algoryx Simulation AB.

#include "Vehicle/AGX_SteeringComponentCustomization.h"

// AGX includes
#include "Vehicle/AGX_AckermannSteeringParameters.h"
#include "Vehicle/AGX_BellCrankSteeringParameters.h"
#include "Vehicle/AGX_DavisSteeringParameters.h"
#include "Vehicle/AGX_RackPinionSteeringParameters.h"
#include "Vehicle/AGX_SteeringComponent.h"
#include "Vehicle/AGX_SteeringParameters.h"

// Unreal includes
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Text/STextBlock.h"
#include "PropertyHandle.h"
#include "Engine/Blueprint.h"

#define LOCTEXT_NAMESPACE "FAGX_SteeringComponentCustomization"

TSharedRef<IDetailCustomization> FAGX_SteeringComponentCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_SteeringComponentCustomization);
}

namespace AGX_SteeringComponentCustomization_helpers
{
	FString GetSteeringParametersName(UAGX_SteeringParameters* Params)
	{
		if (Params == nullptr)
			return "None";
		if (Params->IsA<UAGX_AckermannSteeringParameters>())
			return "Ackermann";
		if (Params->IsA<UAGX_BellCrankSteeringParameters>())
			return "BellCrank";
		if (Params->IsA<UAGX_DavisSteeringParameters>())
			return "Davis";
		if (Params->IsA<UAGX_RackPinionSteeringParameters>())
			return "RackPinon";

		return "Unknown";
	}

	FString GetSteeringParametersTooltip(UAGX_SteeringParameters* Params)
	{
		if (Params == nullptr)
		{
			return "Create a steering parameters Asset and assign it below, for the type of "
				   "steering to be used.";
		}
		if (Params->IsA<UAGX_AckermannSteeringParameters>())
			return "Ackermann steering is used.";
		if (Params->IsA<UAGX_BellCrankSteeringParameters>())
			return "BellCrank steering is used";
		if (Params->IsA<UAGX_DavisSteeringParameters>())
			return "Davis steering is used";
		if (Params->IsA<UAGX_RackPinionSteeringParameters>())
			return "RackPinon steering is used";

		return "Unknown steering type used.";
	}
}

void FAGX_SteeringComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	using namespace AGX_SteeringComponentCustomization_helpers;

	// Get property handle
	TSharedRef<IPropertyHandle> SteeringParamsHandle = DetailBuilder.GetProperty(
		TEXT("SteeringParameters"), UAGX_SteeringComponent::StaticClass());

	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("AGX Steering");

	// Get objects being customized
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);

	const bool bMultipleSelected = Objects.Num() > 1;
	UAGX_SteeringComponent* SteeringComp = nullptr;
	if (!bMultipleSelected && Objects.Num() == 1)
	{
		SteeringComp = Cast<UAGX_SteeringComponent>(Objects[0].Get());
	}

	FString NameString;
	FString TooltipString;

	if (bMultipleSelected)
	{
		NameString = TEXT("Multiple Components Selected");
		TooltipString = TEXT("Multiple components are selected. Steering type cannot be shown.");
	}
	else
	{
		UAGX_SteeringParameters* Params = SteeringComp ? SteeringComp->SteeringParameters : nullptr;

		NameString = GetSteeringParametersName(Params);
		TooltipString = GetSteeringParametersTooltip(Params);
	}

	// clang-format off
	Category.AddCustomRow(LOCTEXT("SteeringTypeSearch", "Steering Type"))
	.NameContent()
	[
		SNew(STextBlock)
		.Text(LOCTEXT("SteeringTypeLabel", "Steering Type"))
		.Font(IDetailLayoutBuilder::GetDetailFontBold())
	]
	.ValueContent()
	[
		SNew(STextBlock)
		.Text(FText::FromString(NameString))
		.ToolTipText(FText::FromString(TooltipString))
		.Font(IDetailLayoutBuilder::GetDetailFont())
	];
	// clang-format on

	Category.AddProperty(SteeringParamsHandle);

	SteeringParamsHandle->SetOnPropertyValueChanged(
		FSimpleDelegate::CreateLambda([&DetailBuilder]() { DetailBuilder.ForceRefreshDetails(); }));
}


#undef LOCTEXT_NAMESPACE
