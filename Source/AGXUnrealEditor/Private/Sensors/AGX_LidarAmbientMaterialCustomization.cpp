// Copyright 2025, Algoryx Simulation AB.

#include "Sensors/AGX_LidarAmbientMaterialCustomization.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_RealInterface.h"
#include "Sensors/AGX_LidarAmbientMaterial.h"
#include "Sensors/RtAmbientMaterialBarrier.h"
#include "Sensors/SensorEnvironmentBarrier.h"
#include "Utilities/AGX_NotificationUtilities.h"

// Unreal Engine includes.
#include "Components/SpinBox.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FAGX_LidarAmbientMaterialCustomization"

TSharedRef<IDetailCustomization> FAGX_LidarAmbientMaterialCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_LidarAmbientMaterialCustomization);
}

void FAGX_LidarAmbientMaterialCustomization::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	DetailBuilder = &InDetailBuilder;

	IDetailCategoryBuilder& CategoryBuilder = InDetailBuilder.EditCategory(
		"Configuration", FText::GetEmpty(), ECategoryPriority::Uncommon);

	// clang-format off

	// Configure as Air.
	CategoryBuilder.AddCustomRow(LOCTEXT("AirConfig", "Air Config"))
	.WholeRowContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(2)
		[
			SNew(SButton)
			.Text(LOCTEXT("ConfigureAsAir", "Configure as Air"))
			.ToolTipText(LOCTEXT("ConfigureAsAirTooltip", "Configure this Material as Air with the set "
				"visibility (kilometers). The Material parameters will update accordingly."))
			.OnClicked(this, &FAGX_LidarAmbientMaterialCustomization::OnConfigureAsAirButtonClicked)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(16, 2, 2, 2) // Padding from button
		[
			SNew(STextBlock)
			.Text(LOCTEXT("AirVisibilityLabel", "Visibility:"))
			.Justification(ETextJustify::Right)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SSpinBox<double>)
				.TypeInterface(MakeShareable(new FAGX_RealInterface))
				.MinValue(0.0)
				.MinDesiredWidth(80)
				.Value_Lambda([this]() { return AirVisibility; })
				.OnValueCommitted_Lambda([this](double InValue, ETextCommit::Type) { AirVisibility = InValue; })
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4, 0)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("AirKmUnit", "km"))
			]
		]
	];

	// Configure as Fog.
	CategoryBuilder.AddCustomRow(LOCTEXT("FogConfig", "Fog Config"))
	.WholeRowContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(2)
		[
			SNew(SButton)
			.Text(LOCTEXT("ConfigureAsFog", "Configure as Fog"))
			.ToolTipText(LOCTEXT("ConfigureAsFogTooltip", "Configure this Material as Fog with the set "
				"visibility (kilometers) and Lidar wavelength (nanometers). "
				"The Material parameters will update accordingly."))
			.OnClicked(this, &FAGX_LidarAmbientMaterialCustomization::OnConfigureAsFogButtonClicked)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(16, 2, 2, 2)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("FogVisibilityLabel", "Visibility:"))
			.Justification(ETextJustify::Right)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SSpinBox<double>)
				.TypeInterface(MakeShareable(new FAGX_RealInterface))
				.MinValue(0.0)
				.MinDesiredWidth(80)
				.Value_Lambda([this]() { return FogVisibility; })
				.OnValueCommitted_Lambda([this](double InValue, ETextCommit::Type) { FogVisibility = InValue; })
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4, 0)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("FogKmUnit", "km"))
			]
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(16, 2, 2, 2)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("FogWavelengthLabel", "Wavelength:"))
			.Justification(ETextJustify::Right)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SSpinBox<double>)
				.TypeInterface(MakeShareable(new FAGX_RealInterface))
				.MinValue(0.0)
				.MinDesiredWidth(80)
				.Value_Lambda([this]() { return FogWavelength; })
				.OnValueCommitted_Lambda([this](double InValue, ETextCommit::Type) { FogWavelength = InValue; })
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4, 0)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("FogNmUnit", "nm"))
			]
		]
	];

	// Configure as Rainfall.
	CategoryBuilder.AddCustomRow(LOCTEXT("RainfallConfig", "Rainfall Config"))
	.WholeRowContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(2)
		[
			SNew(SButton)
			.Text(LOCTEXT("ConfigureAsRainfall", "Configure as Rainfall"))
			.ToolTipText(LOCTEXT("ConfigureAsRainfallTooltip", "Configure this Material to simulate rainfall "
				"based on the rain rate (millimeters per hour)."))
			.OnClicked(this, &FAGX_LidarAmbientMaterialCustomization::OnConfigureAsRainfallButtonClicked)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(16, 2, 2, 2)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("RainRateLabel", "Rain rate:"))
			.Justification(ETextJustify::Right)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SSpinBox<double>)
				.TypeInterface(MakeShareable(new FAGX_RealInterface))
				.MinValue(0.0)
				.MinDesiredWidth(80)
				.Value_Lambda([this]() { return RainRate; })
				.OnValueCommitted_Lambda([this](double InValue, ETextCommit::Type) { RainRate = InValue; })
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4, 0)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("RainRateUnit", "mm/h"))
			]
		]
	];

	// Configure as Snowfall.
	CategoryBuilder.AddCustomRow(LOCTEXT("SnowfallConfig", "Snowfall Config"))
	.WholeRowContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(2)
		[
			SNew(SButton)
			.Text(LOCTEXT("ConfigureAsSnowfall", "Configure as Snowfall"))
			.ToolTipText(LOCTEXT("ConfigureAsSnowfallTooltip", "Configure this Material to simulate snowfall "
				"with snowfall rate (millimeters per hour) and Lidar wavelength (nanometers)."))
			.OnClicked(this, &FAGX_LidarAmbientMaterialCustomization::OnConfigureAsSnowfallButtonClicked)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(16, 2, 2, 2)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("SnowRateLabel", "Snow rate:"))
			.Justification(ETextJustify::Right)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SSpinBox<double>)
				.TypeInterface(MakeShareable(new FAGX_RealInterface))
				.MinValue(0.0)
				.MinDesiredWidth(80)
				.Value_Lambda([this]() { return SnowfallRate; })
				.OnValueCommitted_Lambda([this](double InValue, ETextCommit::Type) { SnowfallRate = InValue; })
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4, 0)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SnowRateUnit", "mm/h"))
			]
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(16, 2, 2, 2)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("SnowWavelengthLabel", "Wavelength:"))
			.Justification(ETextJustify::Right)
		]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SSpinBox<double>)
				.TypeInterface(MakeShareable(new FAGX_RealInterface))
				.MinValue(0.0)
				.MinDesiredWidth(80)
				.Value_Lambda([this]() { return SnowfallWavelength; })
				.OnValueCommitted_Lambda([this](double InValue, ETextCommit::Type) { SnowfallWavelength = InValue; })
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4, 0)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("SnowWavelengthUnit", "nm"))
			]
		]
	];

	// clang-format on
}

namespace AGX_LidarAmbientMaterial_helpers
{
	template <typename ConfigureFuncT>
	void ConfigureAs(IDetailLayoutBuilder& DetailBuilder, ConfigureFuncT Func)
	{
		if (!FSensorEnvironmentBarrier::IsRaytraceSupported())
		{
			FAGX_NotificationUtilities::ShowNotification(
				"Raytrace is not supported on this computer. The Output Log may contain more "
				"information.",
				SNotificationItem::CS_Fail);
			return;
		}

		FRtAmbientMaterialBarrier Barrier;
		Barrier.AllocateNative();
		Func(Barrier);

		int32 NumCopied = 0;
		TArray<TWeakObjectPtr<UObject>> Objects;
		DetailBuilder.GetObjectsBeingCustomized(Objects);
		for (auto& Object : Objects)
		{
			if (auto* Material = Cast<UAGX_LidarAmbientMaterial>(Object.Get()))
			{
				Material->CopyFrom(Barrier);
				NumCopied++;
			}
		}

		if (NumCopied == 0)
		{
			FAGX_NotificationUtilities::ShowNotification(
				"Failed to configure the selected Material.", SNotificationItem::CS_Fail);
		}
		else
		{
			const FString Suffix = NumCopied == 1 ? TEXT("") : TEXT("s");
			FAGX_NotificationUtilities::ShowNotification(
				FString::Printf(TEXT("Successfully configured Material%s."), *Suffix),
				SNotificationItem::CS_Success);
		}
	}
}

FReply FAGX_LidarAmbientMaterialCustomization::OnConfigureAsAirButtonClicked()
{
	using namespace AGX_LidarAmbientMaterial_helpers;
	AGX_CHECK(DetailBuilder != nullptr);
	if (DetailBuilder == nullptr)
		return FReply::Handled();

	ConfigureAs(
		*DetailBuilder,
		[this](FRtAmbientMaterialBarrier& Barrier) { Barrier.ConfigureAsAir(AirVisibility); });
	return FReply::Handled();
}

FReply FAGX_LidarAmbientMaterialCustomization::OnConfigureAsFogButtonClicked()
{
	using namespace AGX_LidarAmbientMaterial_helpers;
	AGX_CHECK(DetailBuilder != nullptr);
	if (DetailBuilder == nullptr)
		return FReply::Handled();

	ConfigureAs(
		*DetailBuilder, [this](FRtAmbientMaterialBarrier& Barrier)
		{ Barrier.ConfigureAsFog(FogVisibility, FogWavelength); });
	return FReply::Handled();
}

FReply FAGX_LidarAmbientMaterialCustomization::OnConfigureAsRainfallButtonClicked()
{
	using namespace AGX_LidarAmbientMaterial_helpers;
	AGX_CHECK(DetailBuilder != nullptr);
	if (DetailBuilder == nullptr)
		return FReply::Handled();

	ConfigureAs(
		*DetailBuilder,
		[this](FRtAmbientMaterialBarrier& Barrier) { Barrier.ConfigureAsRainfall(RainRate); });
	return FReply::Handled();
}

FReply FAGX_LidarAmbientMaterialCustomization::OnConfigureAsSnowfallButtonClicked()
{
	using namespace AGX_LidarAmbientMaterial_helpers;
	AGX_CHECK(DetailBuilder != nullptr);
	if (DetailBuilder == nullptr)
		return FReply::Handled();

	ConfigureAs(
		*DetailBuilder, [this](FRtAmbientMaterialBarrier& Barrier)
		{ Barrier.ConfigureAsSnowfall(SnowfallRate, SnowfallWavelength); });
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
