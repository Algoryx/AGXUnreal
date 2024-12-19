// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

class UBlueprint;

struct FAGX_AGXToUeContext;
struct FAGX_ImporterSettings;

class AGXUNREALEDITOR_API FAGX_ImporterToEditor
{
public:
	/**
	 * Todo: Add comment.
	 */
	UBlueprint* Import(const FAGX_ImporterSettings& Settings);

	/**
	 * Todo: Add comment.
	 */
	bool Reimport(
		UBlueprint& BaseBP, const FAGX_ImporterSettings& Settings,
		UBlueprint* OpenBlueprint = nullptr);

	void UpdateBlueprint(UBlueprint& Blueprint, const FAGX_AGXToUeContext& Context);

private:
	template <typename T>
	T* UpdateOrCreateAsset(T& Source);

	FString RootDirectory;
	FString ModelName;

	// Maps objects owned by transient package (not written to disk) to the corresponding asset.
	// The transient package objects comes from the FAGX_Importer. This map is used during Reimport.
	TMap<UObject*, UObject*> TransientToAsset;
};
