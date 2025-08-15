// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Import/AGX_ImportEnums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

class UBlueprint;

struct FAGX_ImportContext;
struct FAGX_ImportSettings;
struct FAGX_ReimportSettings;
struct FAGX_SCSNodeCollection;

class AGXUNREALEDITOR_API FAGX_ImporterToEditor
{
public:
	/**
	 * Import an .agx archive, OpenPLX or URDF model to a Blueprint.
	 * For OpenPLX imports, it is allowed to pass a path to an OpenPLX file anywhere on the system,
	 * but it will automatically be copied to the projects OpenPLXModels directory prior to being
	 * imported.
	 */
	UBlueprint* Import(FAGX_ImportSettings Settings);

	/**
	 * Reimport an .agx archive or OpenPLX model to an existing Blueprint.
	 */
	bool Reimport(
		UBlueprint& BaseBP, FAGX_ReimportSettings Settings,
		UBlueprint* OpenBlueprint = nullptr);

private:
	EAGX_ImportResult UpdateBlueprint(
		UBlueprint& Blueprint, const FAGX_ReimportSettings& Settings,
		const FAGX_ImportContext& Context);
	EAGX_ImportResult UpdateAssets(UBlueprint& Blueprint, const FAGX_ImportContext& Context);
	EAGX_ImportResult UpdateComponents(
		UBlueprint& Blueprint, const FAGX_ReimportSettings& Settings,
		const FAGX_ImportContext& Context);

	template <typename T>
	T* UpdateOrCreateAsset(T& Source, const FAGX_ImportContext& Context);

	void PreImport(FAGX_ImportSettings& OutSettings);
	void PreReimport(const UBlueprint& Blueprint, FAGX_ImportSettings& OutSettings);
	void PostImport(const FAGX_ImportSettings& Settings);

	FString RootDirectory;
	FString ModelName;

	// Maps objects owned by transient package (not written to disk) to the corresponding asset.
	// The transient package objects comes from the FAGX_Importer. This map is used during Reimport.
	TMap<UObject*, UObject*> TransientToAsset;
};
