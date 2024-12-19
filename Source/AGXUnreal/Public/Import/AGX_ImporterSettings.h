// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

struct FAGX_ImporterSettings
{
	FString FilePath;
	bool bIgnoreDisabledTrimeshes = true;
	bool bOpenBlueprintEditorAfterImport = true; // Todo : remove
};

struct FAGX_AGXImporterSettings : public FAGX_ImporterSettings
{
};

struct FAGX_UrdfImporterSettings : public FAGX_ImporterSettings
{
	// The path to the URDF package directory. Corresponds to the `package://` part of any filepath
	// in the .urdf file, typically used for pointing at mesh files. Can be left empty if the URDF
	// file does not have any file paths in it, or obviously, if ImportType is not Urdf.
	FString UrdfPackagePath;

	TArray<double> UrdfInitialJoints;
};
