// Copyright 2025, Algoryx Simulation AB.

#include "AgxEdMode/AGX_AgxEdModeTerrainCustomization.h"

// AGX Dynamics includes.
#include "AGX_LogCategory.h"
#include "Materials/AGX_MaterialLibrary.h"
#include "Utilities/AGX_NotificationUtilities.h"

// Unreal Engine includes.
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "DetailWidgetRow.h"
#include "Input/Reply.h"
#include "Widgets/Input/SButton.h"

#define LOCTEXT_NAMESPACE "FAGX_AgxEdModeTerrainCustomization"

TSharedRef<IDetailCustomization> FAGX_AgxEdModeTerrainCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_AgxEdModeTerrainCustomization);
}

namespace FAGX_AgxEdModeTerrainCustomization_helpers
{
	bool RefreshTerrainMaterialLibrary()
	{
		return AGX_MaterialLibrary::InitializeTerrainMaterialAssetLibrary(true);
	}
}

void FAGX_AgxEdModeTerrainCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	using namespace FAGX_AgxEdModeTerrainCustomization_helpers;

	IDetailCategoryBuilder& CategoryBuilder =
		DetailBuilder.EditCategory("Terrain Material Library");
	CategoryBuilder.InitiallyCollapsed(false);

	// clang-format off
	CategoryBuilder.AddCustomRow(FText::GetEmpty())
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("btnRefreshTerrainMaterialLibrary", "Refresh Terrain Material Library"))
			.ToolTipText(LOCTEXT("TtRefreshTerrainMaterialLibrary", "Reads all Terrain Materials in the "
				"AGX Dynamics Materials Library and updates the pre-defined Terrain Materials in the "
				"Plugin Contents."))
			.OnClicked_Lambda([]()
			{
				if (RefreshTerrainMaterialLibrary())
				{
					FAGX_NotificationUtilities::ShowNotification(
						"Material Library Updated.", SNotificationItem::CS_Success);
				}
				else
				{
					FAGX_NotificationUtilities::ShowNotification(
						"Issues encountered during Refresh, see the Output Log for more details.",
						SNotificationItem::CS_Fail);
				}
				return FReply::Handled();
			})
		]
	];
	// clang-format on
}

#undef LOCTEXT_NAMESPACE
