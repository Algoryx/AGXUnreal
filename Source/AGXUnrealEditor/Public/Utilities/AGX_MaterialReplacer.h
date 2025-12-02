// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/WeakObjectPtr.h"

class AActor;
class UBlueprint;
class UMaterialInterface;
struct FAssetData;

/**
 * Helper class for replacing render Materials in a Blueprint or Actor.
 *
 * Keeps track of which Material should be replace by which other Material and does the actual
 * replacing. Intended to be used with SObjectPropertyEntryBox.
 */
struct AGXUNREALEDITOR_API FAGX_MaterialReplacer
{
	static void SetCurrent(const FAssetData& AssetData);
	static void SetNew(const FAssetData& AssetData);
	static FString GetCurrentPathName();
	static FString GetNewPathName();

	static bool ReplaceMaterials(UBlueprint& Blueprint);
	static bool ReplaceMaterials(AActor& Actor);

	inline static TWeakObjectPtr<UMaterialInterface> CurrentMaterial;
	inline static TWeakObjectPtr<UMaterialInterface> NewMaterial;
};
