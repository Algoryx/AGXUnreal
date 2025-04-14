// Copyright 2024, Algoryx Simulation AB.

#include "AGX_ArchiveExporter.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Simulation.h"
#include "AGX_LogCategory.h"
#include "Utilities/AGX_EditorUtilities.h"
#include "Utilities/AGX_NotificationUtilities.h"

// Unreal Engine includes.
#include "Engine/World.h"

bool AGX_ArchiveExporter::ExportAGXArchive(const FString& ArchivePath)
{
	UWorld* World = FAGX_EditorUtilities::GetCurrentWorld();
	UAGX_Simulation* Simulation = UAGX_Simulation::GetFrom(World);
	if (Simulation == nullptr)
	{
		UE_LOG(
			LogAGX, Warning,
			TEXT("The current world does not have a simulation. Cannot store AGX Dynamics "
				 "archive."));
		return false;
	}

	const bool Result = Simulation->WriteAGXArchive(ArchivePath);

	if (Result)
	{
		FAGX_NotificationUtilities::ShowNotification(
			FString::Printf(TEXT("Succesfully exported .agx to: '%s'"), *ArchivePath),
			SNotificationItem::CS_Success);
	}
	else
	{
		FAGX_NotificationUtilities::ShowNotification(
			FString::Printf(
				TEXT("Unable to export .agx to: '%s'. The Output Log may contain more "
					 "information."),
				*ArchivePath),
			SNotificationItem::CS_Fail);
	}

	return Result;
}
