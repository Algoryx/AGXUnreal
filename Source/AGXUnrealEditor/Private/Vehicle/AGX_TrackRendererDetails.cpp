// Copyright 2025, Algoryx Simulation AB.

#include "Vehicle/AGX_TrackRendererDetails.h"

// AGX Dynamics for Unreal includes.
#include "Vehicle/AGX_TrackComponent.h"
#include "Vehicle/AGX_TrackRenderer.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "FAGX_TrackRendererDetails"

TSharedRef<IDetailCustomization> FAGX_TrackRendererDetails::MakeInstance()
{
	return MakeShareable(new FAGX_TrackRendererDetails);
}

void FAGX_TrackRendererDetails::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	IDetailCategoryBuilder& CategoryBuilder = InDetailBuilder.EditCategory(
		"AGX Track Renderer", FText::GetEmpty(), ECategoryPriority::Transform);

	TArray<TWeakObjectPtr<UObject>> Objects;
	InDetailBuilder.GetObjectsBeingCustomized(Objects);

	// clang-format off
	CategoryBuilder.AddCustomRow(FText()).WholeRowContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("DeprecationText", "Track Renderer is deprecated."))
				.ColorAndOpacity(FSlateColor(FLinearColor::Red))
			]
	];

	CategoryBuilder.AddCustomRow(FText()).WholeRowContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("UpdatePreviewText", "Update Visual"))
			.OnClicked_Lambda([Objects]()
			{
				for (const TWeakObjectPtr<UObject>& Object : Objects)
				{
					auto Renderer = Cast<UAGX_TrackRenderer>(Object.Get());
					if (IsValid(Renderer))
					{
						if (UAGX_TrackComponent* Track = Renderer->FindTargetTrack())
						{
							// Mark Track Preview Data for update.
							Track->RaiseTrackPreviewNeedsUpdate();
						}
						// Get updated Track Preview Data and synchronize renderer data.
						Renderer->RebindToTrackPreviewNeedsUpdateEvent(false);
						Renderer->SynchronizeVisuals();
					}
				}
				return FReply::Handled();
			})
		]
	];
	// clang-format on
}

#undef LOCTEXT_NAMESPACE
