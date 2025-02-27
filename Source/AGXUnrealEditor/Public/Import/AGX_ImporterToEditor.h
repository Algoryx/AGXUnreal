// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Import/AGX_ImportEnums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

class UBlueprint;

struct FAGX_ImportContext;
struct FAGX_ImportSettings;
struct FAGX_ReimportSettings;

class AGXUNREALEDITOR_API FAGX_ImporterToEditor
{
public:
	/**
	 * Todo: Add comment.
	 */
	UBlueprint* Import(const FAGX_ImportSettings& Settings);

	/**
	 * Todo: Add comment.
	 */
	bool Reimport(
		UBlueprint& BaseBP, const FAGX_ReimportSettings& Settings,
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

	FString RootDirectory;
	FString ModelName;

	// Maps objects owned by transient package (not written to disk) to the corresponding asset.
	// The transient package objects comes from the FAGX_Importer. This map is used during Reimport.
	TMap<UObject*, UObject*> TransientToAsset;
};
