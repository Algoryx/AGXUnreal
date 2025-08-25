// Copyright 2025, Algoryx Simulation AB.

#include "AgxEdMode/AGX_AgxEdModeFile.h"

// AGX Dynamics for Unreal includes.
#include "AGX_ArchiveExporter.h"
#include "AGX_EditorStyle.h"
#include "Import/AGX_ImportSettings.h"
#include "AGX_LogCategory.h"
#include "AGX_Simulation.h"
#include "Import/AGX_ImportSettings.h"
#include "Import/AGX_ImporterToEditor.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"
#include "Widgets/AGX_ImportDialog.h"

// Unreal Engine includes.
#include "DesktopPlatformModule.h"
#include "Engine/Blueprint.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Textures/SlateIcon.h"

#define LOCTEXT_NAMESPACE "UAGX_AgxEdModeFile"

UAGX_AgxEdModeFile* UAGX_AgxEdModeFile::GetInstance()
{
	static UAGX_AgxEdModeFile* FileTool = nullptr;

	if (FileTool == nullptr)
	{
		FileTool = GetMutableDefault<UAGX_AgxEdModeFile>();
	}

	return FileTool;
}

void UAGX_AgxEdModeFile::ImportToBlueprint()
{
	TSharedRef<SWindow> Window =
		SNew(SWindow)
			.SupportsMinimize(false)
			.SupportsMaximize(false)
			.SizingRule(ESizingRule::Autosized)
			.Title(NSLOCTEXT("AGX", "AGXUnrealImport", "Import AGX Dynamics, OpenPLX or URDF file"));

	TSharedRef<SAGX_ImportDialog> ImportDialog = SNew(SAGX_ImportDialog);
	Window->SetContent(ImportDialog);
	FSlateApplication::Get().AddModalWindow(Window, nullptr);

	if (auto Settings = ImportDialog->ToImportSettings())
	{
		FAGX_ImporterToEditor Importer;
		Importer.Import(*Settings);
	}
}

void UAGX_AgxEdModeFile::ExportAgxArchive()
{
	UWorld* World = FAGX_EditorUtilities::GetCurrentWorld();
	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(World);
	if (Simulation == nullptr || !Simulation->HasNative())
	{
		FAGX_NotificationUtilities::ShowNotification(
			"Cannot export Simulation contents to an AGX Dynamics archive because no Simulation is "
			"active. Launch a Play-In-Editor session.",
			SNotificationItem::CS_Fail);
		return;
	}

	FString Filename = FAGX_EditorUtilities::SelectNewFileDialog(
		"Select an AGX Archive to export", "AGX Dynamics Archive|*.agx", "unreal.agx", "");

	if (Filename.IsEmpty())
	{
		return; // Logging done in FAGX_EditorUtilities::SelectNewFileDialog().
	}

	FString Extension = FPaths::GetExtension(Filename);
	if (Extension != "agx" && Extension != "aagx")
	{
		Filename += ".agx";
	}

	if (AGX_ArchiveExporter::ExportAGXArchive(Filename))
	{
		UE_LOG(LogAGX, Log, TEXT("AGX Dynamics archive saved to %s."), *Filename);
	}
	else
	{
		UE_LOG(LogAGX, Warning, TEXT("AGX Dynamics archive could not be saved to %s."), *Filename);
	}
}

void UAGX_AgxEdModeFile::ReimportModel_BP(UObject* Bp)
{
	UBlueprint* Blueprint = Cast<UBlueprint>(Bp);
	if (Blueprint == nullptr )
		return;

	FAGX_EditorUtilities::ReimportModel(*Blueprint, false);
}

void UAGX_AgxEdModeFile::SynchronizeModel_BP(UObject* Bp)
{
	ReimportModel_BP(Bp);
}

FText UAGX_AgxEdModeFile::GetDisplayName() const
{
	return LOCTEXT("DisplayName", "File");
}

FText UAGX_AgxEdModeFile::GetTooltip() const
{
	return LOCTEXT(
		"Tooltip",
		"Interoperability with external file formats, such AGX simulation files (.agx) "
		"or URDF (.urdf) files.");
}

FSlateIcon UAGX_AgxEdModeFile::GetIcon() const
{
	static FSlateIcon Icon(
		FAGX_EditorStyle::GetStyleSetName(), FAGX_EditorStyle::FileIcon,
		FAGX_EditorStyle::FileIconSmall);
	return Icon;
}

#undef LOCTEXT_NAMESPACE
