// Copyright 2025, Algoryx Simulation AB.

#include "AgxEdMode/AGX_AgxEdModeFileCustomization.h"

// AGX Dynamics for Unreal includes.
#include "Materials/AGX_MaterialLibrary.h"
#include "AgxEdMode/AGX_AgxEdModeFile.h"
#include "Utilities/AGX_NotificationUtilities.h"

// Unreal Engine includes.
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"

#define LOCTEXT_NAMESPACE "FAGX_AgxEdModeFileCustomization"

TSharedRef<IDetailCustomization> FAGX_AgxEdModeFileCustomization::MakeInstance()
{
	return MakeShareable(new FAGX_AgxEdModeFileCustomization);
}

void FAGX_AgxEdModeFileCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	CustomizeFileImporterCategory(DetailBuilder);
	CustomizeFileExporterCategory(DetailBuilder);
	CustomizeMaterialLibraryCategory(DetailBuilder);
}

void FAGX_AgxEdModeFileCustomization::CustomizeFileImporterCategory(
	IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& CategoryBuilder = DetailBuilder.EditCategory("AGX File Importer");
	CategoryBuilder.InitiallyCollapsed(false);

	// Create import Buttons.

	AddCustomButton(
		CategoryBuilder, LOCTEXT("CreateButtonTextImportBP", "Import Model to Blueprint..."),
		LOCTEXT("CreateButtonTextImportTt", "Import a model from a file to a Blueprint."),
		[&]()
		{
			UAGX_AgxEdModeFile::ImportToBlueprint();
			return FReply::Handled();
		});
}

void FAGX_AgxEdModeFileCustomization::CustomizeFileExporterCategory(
	IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& CategoryBuilder = DetailBuilder.EditCategory("Export AGX Archive");

	CategoryBuilder.InitiallyCollapsed(false);

	/** Create AGX Archive export Button */
	AddCustomButton(
		CategoryBuilder, LOCTEXT("CreateButtonTextEx", "Export AGX Archive..."),
		LOCTEXT(
			"CreateButtonTextTt", "Export the current Simulation state to an AGX Archive (.agx)."),
		[&]()
		{
			UAGX_AgxEdModeFile::ExportAgxArchive();
			return FReply::Handled();
		});
}

namespace AGX_AgxEdModeFileCustomization_helpers
{
	bool RefreshMaterialLibraries()
	{
		bool Success = true;
		Success &= AGX_MaterialLibrary::InitializeShapeMaterialAssetLibrary(true);
		Success &= AGX_MaterialLibrary::InitializeContactMaterialAssetLibrary(true);
		Success &= AGX_MaterialLibrary::InitializeTerrainMaterialAssetLibrary(true);
		Success &= AGX_MaterialLibrary::InitializeLidarAmbientMaterialAssetLibrary(true);
		return Success;
	}
}

void FAGX_AgxEdModeFileCustomization::CustomizeMaterialLibraryCategory(
	IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& CategoryBuilder = DetailBuilder.EditCategory("Material Library");

	CategoryBuilder.InitiallyCollapsed(false);

	/** Create AGX Naterial Libraries refresh Button */
	AddCustomButton(
		CategoryBuilder, LOCTEXT("RefreshMaterialLibraryEx", "Refresh Material Libraries"),
		LOCTEXT(
			"RefreshMaterialLibraryTt",
			"Reads all Materials in the AGX Dynamics Materials "
			"Library and updates the pre-defined Materials in the Plugin Contents."),
		[&]()
		{
			if (AGX_AgxEdModeFileCustomization_helpers::RefreshMaterialLibraries())
			{
				FAGX_NotificationUtilities::ShowNotification(
					"Material Library Updated.", SNotificationItem::CS_Success);
			}
			else
			{
				FAGX_NotificationUtilities::ShowNotification(
					"Issues encountered during Refresh, see the Console Log for more details.",
					SNotificationItem::CS_Fail);
			}
			return FReply::Handled();
		});
}

#undef LOCTEXT_NAMESPACE
