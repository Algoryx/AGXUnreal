// Copyright 2024, Algoryx Simulation AB.

#pragma once

struct FAssetData;

struct AGXUNREALEDITOR_API FAGX_MaterialReplacer
{
	static void SetCurrent(const FAssetData& AssetData);
	static void SetNew(const FAssetData& AssetData);
	static FString GetCurrentPathName();
	static FString GetNewPathName();

	static bool ReplaceMaterials(UBlueprint& EditBlueprint, IPropertyHandle& PropertyHandle);
	static bool ReplaceMaterials(AActor& Actor);

	inline static TWeakObjectPtr<UMaterialInterface> CurrentMaterial;
	inline static TWeakObjectPtr<UMaterialInterface> NewMaterial;
};
