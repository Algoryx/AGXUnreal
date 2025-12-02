// Copyright 2025, Algoryx Simulation AB.

#include "CollisionGroups/AGX_CollisionGroupAdderComponentCustomization.h"

// AGX Dynamics for Unreal includes.
#include "CollisionGroups/AGX_CollisionGroupAdderComponent.h"
#include "Utilities/AGX_EditorUtilities.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "FAGX_CollisionGroupAdderComponentCustomization"

TSharedRef<IDetailCustomization> FAGX_CollisionGroupAdderComponentCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_CollisionGroupAdderComponentCustomization);
}

void FAGX_CollisionGroupAdderComponentCustomization::CustomizeDetails(
	IDetailLayoutBuilder& DetailBuilder)
{
	UAGX_CollisionGroupAdderComponent* CollisionGroupComponent =
		FAGX_EditorUtilities::GetSingleObjectBeingCustomized<UAGX_CollisionGroupAdderComponent>(
			DetailBuilder);

	if (!CollisionGroupComponent)
	{
		return;
	}

	IDetailCategoryBuilder& CategoryBuilder = DetailBuilder.EditCategory("AGX Collision Groups");

	CategoryBuilder.AddProperty(DetailBuilder.GetProperty(
		GET_MEMBER_NAME_CHECKED(UAGX_CollisionGroupAdderComponent, CollisionGroups)));

	// clang-format off

	// Add button for forcing a refresh of all child shape components according
	// to collision groups list.
	CategoryBuilder.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("CreateCollisionForceRefreshButtonText", "Force refresh all shapes"))
			.ToolTipText(LOCTEXT(
				"CollisionForceRefreshButtonTooltip",
				"Force apply collision groups to all child shape components."))
			.OnClicked_Lambda([CollisionGroupComponent]()
			{
				CollisionGroupComponent->ForceRefreshChildShapes();
				return FReply::Handled();
			})
		]
	];

	// clang-format on

	// Hide CollisionGroupsLastChange from the Editor view
	DetailBuilder.HideProperty(DetailBuilder.GetProperty(
		GET_MEMBER_NAME_CHECKED(UAGX_CollisionGroupAdderComponent, CollisionGroupsLastChange)));
}

#undef LOCTEXT_NAMESPACE
