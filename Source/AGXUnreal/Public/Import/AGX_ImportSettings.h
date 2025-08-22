// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Import/AGX_ImportEnums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ImportSettings.generated.h"

USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_ImportSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Import")
	EAGX_ImportType ImportType = EAGX_ImportType::Invalid;

	// #if AGXUNREAL_USE_OPENPLX
	// Reminder: Re-add OpenPLX below.

	/**
	 * Absolute file path to the .agx archive or Urdf file to be imported.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Import")
	FString FilePath;

	/**
	 * Recommended for large models.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Import")
	bool bIgnoreDisabledTrimeshes = true;

	/**
	 * Only relevant when importing to Blueprint.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Import")
	bool bOpenBlueprintEditorAfterImport = true;

	/**
	 * Only relevant for URDF files.
	 * The path to the URDF package directory. Corresponds to the `package` part of any filepath
	 * in the .urdf file, typically used for pointing at mesh files. Can be left empty if the
	 * URDF file does not have any file paths in it, or obviously, if ImportType is not Urdf.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Import")
	FString UrdfPackagePath;

	/**
	 * Only relevant for Urdf files.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Import")
	TArray<double> UrdfInitialJoints;

	// #if AGXUNREAL_USE_OPENPLX
	// Reminder: Re-add OpenPLX below, and text about differing paths for copied OpenPLX models.

	/**
	 * Absolute file path to the original .agx archive or Urdf file that was selected for
	 * Import or Reimport.
	 */
	UPROPERTY()
	FString SourceFilePath;

	UPROPERTY(EditAnywhere, Category = "AGX Reimport Model Info")
	bool bRuntimeImport {false};
};

struct FAGX_ReimportSettings : public FAGX_ImportSettings
{
	bool bForceOverwriteProperties = false;
	bool bForceReassignRenderMaterials = false;
};
