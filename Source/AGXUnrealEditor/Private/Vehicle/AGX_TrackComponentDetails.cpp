// Copyright 2025, Algoryx Simulation AB.

#include "Vehicle/AGX_TrackComponentDetails.h"

// AGX Dynamics for Unreal includes.
#include "Vehicle/AGX_TrackComponent.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Engine/World.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "FAGX_TrackComponentDetails"

TSharedRef<IDetailCustomization> FAGX_TrackComponentDetails::MakeInstance()
{
	return MakeShareable(new FAGX_TrackComponentDetails);
}

void FAGX_TrackComponentDetails::CustomizeDetails(IDetailLayoutBuilder& InDetailBuilder)
{
	IDetailCategoryBuilder& CategoryBuilder =
		InDetailBuilder.EditCategory("AGX Track Debug Visual");

	TArray<TWeakObjectPtr<UObject>> Objects;
	InDetailBuilder.GetObjectsBeingCustomized(Objects);

	// clang-format off
	CategoryBuilder.AddCustomRow(FText()).WholeRowContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("UpdatePreviewText", "Update Preview Data"))
			.OnClicked_Lambda([Objects]()
			{
				for (const TWeakObjectPtr<UObject>& Object : Objects)
				{
					UAGX_TrackComponent* Track = Cast<UAGX_TrackComponent>(Object.Get());
					if (IsValid(Track))
					{
						// Mark Track Preview Data for update.
						Track->RaiseTrackPreviewNeedsUpdate();
					}
				}
				return FReply::Handled();
			})
			.IsEnabled_Lambda([Objects]()
			{
				if (Objects.Num() == 0)
					return false;
				UObject* Obj = Objects[0].Get();
				bool bIsPlaying = Obj && Obj->GetWorld() && Obj->GetWorld()->IsGameWorld();
				return !bIsPlaying;
			})
		]
	];
	// clang-format on
}

#undef LOCTEXT_NAMESPACE
