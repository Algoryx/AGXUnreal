// Copyright 2026, Algoryx Simulation AB.

#include "AgxEdMode/AGX_AgxEdModeTerrainCustomization.h"

// AGX Dynamics includes.
#include "AGX_LogCategory.h"
#include "Materials/AGX_MaterialLibrary.h"
#include "Terrain/AGX_LandscapeMaterialLibrary.h"
#include "Terrain/AGX_Terrain.h"
#include "Utilities/AGX_NotificationUtilities.h"

// Unreal Engine includes.
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Editor.h"
#include "Engine/Selection.h"
#include "Input/Reply.h"
#include "Landscape.h"
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
		return AGX_MaterialLibrary::UpdateTerrainMaterialAssetLibrary();
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
	IDetailCategoryBuilder& Deformation = DetailBuilder.EditCategory("Deformable Landscape");
	Deformation.AddCustomRow(LOCTEXT("DeformableNanite", "Deformable Nanite"))
		.WholeRowContent()
			[SNew(SButton)
				 .Text(LOCTEXT("PrepareDeformableNanite", "Prepare and Save Deformable Nanite"))
				 .ToolTipText(LOCTEXT(
					 "PrepareDeformableNaniteTooltip",
					 "Prepare vertical displacement normals for selected Terrain sources or "
					 "Landscape proxies. "
					 "Saves affected meshes and proxy packages. World Partition Terrain sources "
					 "include unloaded proxies."))
				 .IsEnabled_Lambda([]()
								   { return GEditor != nullptr && GEditor->PlayWorld == nullptr; })
				 .OnClicked_Lambda(
					 []()
					 {
						 TSet<ALandscapeProxy*> Sources;
						 for (FSelectionIterator It(*GEditor->GetSelectedActors()); It; ++It)
						 {
							 if (auto* Terrain = Cast<AAGX_Terrain>(*It))
							 {
								 if (Terrain->SourceLandscape != nullptr)
									 Sources.Add(Terrain->SourceLandscape);
							 }
							 else if (auto* Proxy = Cast<ALandscapeProxy>(*It))
								 Sources.Add(Proxy);
						 }
						 bool bSuccess = !Sources.IsEmpty();
						 for (ALandscapeProxy* Source : Sources)
							 bSuccess &= UAGX_LandscapeMaterialLibrary::BuildLandscapeNanite(
								 Source, true, true);
						 FAGX_NotificationUtilities::ShowNotification(
							 bSuccess ? TEXT("Deformable Nanite data prepared and saved.")
									  : TEXT(
											"Select a Terrain or Nanite Landscape. Preparation "
											"requires Nanite enabled; see Output Log."),
							 bSuccess ? SNotificationItem::CS_Success : SNotificationItem::CS_Fail);
						 return FReply::Handled();
					 })];
}

#undef LOCTEXT_NAMESPACE
