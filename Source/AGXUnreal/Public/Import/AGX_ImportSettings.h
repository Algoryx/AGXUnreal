// Copyright 2024, Algoryx Simulation AB.

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

	/**
	* Absolute file path to the .agx archive, OpenPLX or Urdf file.
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
	 * Only relevant for Urdf files.
	 * The path to the URDF package directory.Corresponds to the `package` part of any filepath
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
};

struct FAGX_ReimportSettings : public FAGX_ImportSettings
{
	bool bForceOverwriteProperties = false;
	bool bForceReassignRenderMaterials = false;
};
