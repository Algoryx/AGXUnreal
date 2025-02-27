// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Import/AGX_ImportEnums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

struct FAGX_ImportSettings
{
	EAGX_ImportType ImportType = EAGX_ImportType::Invalid;
	FString FilePath;
	bool bIgnoreDisabledTrimeshes = true;
	bool bOpenBlueprintEditorAfterImport = true;

	// The path to the URDF package directory. Corresponds to the `package://` part of any filepath
	// in the .urdf file, typically used for pointing at mesh files. Can be left empty if the URDF
	// file does not have any file paths in it, or obviously, if ImportType is not Urdf.
	FString UrdfPackagePath;

	TArray<double> UrdfInitialJoints;
};


struct FAGX_ReimportSettings : public FAGX_ImportSettings
{
	bool bForceOverwriteProperties = false;
	bool bForceReassignRenderMaterials = false;
};

