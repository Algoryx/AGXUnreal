// Copyright 2025, Algoryx Simulation AB.

#include "Plot/AGX_PlotComponentCustomization.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "Plot/AGX_PlotComponent.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "FAGX_PlotComponentCustomization"

TSharedRef<IDetailCustomization> FAGX_PlotComponentCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_PlotComponentCustomization);
}

void FAGX_PlotComponentCustomization::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	DetailBuilder = &InDetailBuilder;

	const UAGX_PlotComponent* PlotComponent =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_PlotComponent>(InDetailBuilder);
	if (PlotComponent == nullptr)
	{
		return;
	}

	IDetailCategoryBuilder& CategoryBuilder = InDetailBuilder.EditCategory("AGX Plot");

	// clang-format off
	CategoryBuilder.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("OpenPlotWindowButtonText", "Open Plot Window"))
			.ToolTipText(LOCTEXT(
				"OpenPlotWindowTooltip",
				"Open the default Web Browser showing the Plots managed by this Component."))
			.OnClicked(this, &FAGX_PlotComponentCustomization::OnOpenPlotWindowButtonClicked)
		]
	];
	// clang-format on
}

FReply FAGX_PlotComponentCustomization::OnOpenPlotWindowButtonClicked()
{
	AGX_CHECK(DetailBuilder);

	UAGX_PlotComponent* PlotComponent =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_PlotComponent>(*DetailBuilder);
	if (!PlotComponent)
	{
		return FReply::Handled();
	}

	if (!PlotComponent->HasNative())
	{
		FAGX_NotificationUtilities::ShowNotification(
			"Open Plot Window is only possible during Play.", SNotificationItem::CS_Fail, 3.f);
		return FReply::Handled();
	}

	PlotComponent->OpenPlotWindow();
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
