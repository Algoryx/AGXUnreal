// Copyright 2025, Algoryx Simulation AB.

#include "Materials/MaterialLibraryBarrier.h"

// AGX Dynamics for Unreal includes.
#include <AGX_Environment.h>
#include <AGXBarrierFactories.h>
#include <TypeConversions.h>

// AGX Dynamics includes.
#include <BeginAGXIncludes.h>
#include <agx/MaterialLibrary.h>
#include <agx/MaterialReaderWriter.h>
#include <agxTerrain/TerrainMaterial.h>
#include <agxTerrain/TerrainMaterialLibrary.h>
#include <agxUtil/agxUtil.h>
#include <EndAGXIncludes.h>

// Unreal Engine includes.
#include "Misc/Paths.h"

namespace MaterialLibraryBarrier_helpers
{
	TArray<FString> GetAvailableLibraryMaterialsAny(agx::MaterialLibrary::LibraryItemType Type)
	{
		agx::StringVector NamesAGX = agx::MaterialLibrary::getAvailableItems(Type);
		TArray<FString> NamesUnreal;
		NamesUnreal.Reserve(NamesAGX.size());
		for (const agx::String& NameAGX : NamesAGX)
		{
			NamesUnreal.Add(Convert(NameAGX));
		}

		// Must be called to avoid crash due to different allocators used by AGX Dynamics and
		// Unreal Engine.
		agxUtil::freeContainerMemory(NamesAGX);

		return NamesUnreal;
	}
}

AGXUNREALBARRIER_API TArray<FString>
AGX_MaterialLibraryBarrier::GetAvailableLibraryContactMaterials()
{
	return MaterialLibraryBarrier_helpers::GetAvailableLibraryMaterialsAny(
		agx::MaterialLibrary::CONTACT_MATERIAL);
}

AGXUNREALBARRIER_API TOptional<FContactMaterialBarrier>
AGX_MaterialLibraryBarrier::LoadContactMaterialProfile(const FString& MaterialName)
{
	// There is no convenient function in AGX for getting Contact Materials from name, so we must
	// go via the json file directly.
	const FString RelFilePathJson =
		FPaths::Combine("MaterialLibrary", "ContactMaterials", MaterialName + FString(".json"));

	const FString FullPathJson = FAGX_Environment::FindAGXEnvironmentResourcePath(RelFilePathJson);
	if (!FPaths::FileExists(FullPathJson))
	{
		UE_LOG(
			LogAGX, Warning, TEXT("Could not find Contact Material '%s' in path '%s'."),
			*MaterialName, *FullPathJson);
		return {};
	}

	agx::ContactMaterialRef Cm = new agx::ContactMaterial(nullptr, nullptr);
	if (!agx::MaterialReaderWriter::readJson(Cm, Convert(FullPathJson)))
	{
		UE_LOG(
			LogAGX, Warning, TEXT("Could not create Contact Material from Json file '%s'."),
			*FullPathJson);
		return {};
	}

	return AGXBarrierFactories::CreateContactMaterialBarrier(Cm);
}

TArray<FString> AGX_MaterialLibraryBarrier::GetAvailableLibraryShapeMaterials()
{
	return MaterialLibraryBarrier_helpers::GetAvailableLibraryMaterialsAny(
		agx::MaterialLibrary::MATERIAL);
}

TOptional<FShapeMaterialBarrier> AGX_MaterialLibraryBarrier::LoadShapeMaterialProfile(
	const FString& MaterialName)
{
	const agx::String MaterialNameAGX = Convert(MaterialName);
	agx::MaterialRef Material = agx::MaterialLibrary::loadMaterial(MaterialNameAGX);
	if (Material == nullptr)
		return {};

	return AGXBarrierFactories::CreateShapeMaterialBarrier(Material);
}

AGXUNREALBARRIER_API TArray<FString>
AGX_MaterialLibraryBarrier::GetAvailableLibraryTerrainMaterials()
{
	return MaterialLibraryBarrier_helpers::GetAvailableLibraryMaterialsAny(
		agx::MaterialLibrary::TERRAIN_MATERIAL);
}

AGXUNREALBARRIER_API TOptional<FTerrainMaterialBarrier>
AGX_MaterialLibraryBarrier::LoadTerrainMaterialProfile(const FString& MaterialName)
{
	const agx::String MaterialNameAGX = Convert(MaterialName);
	agxTerrain::TerrainMaterialRef Material = new agxTerrain::TerrainMaterial(MaterialNameAGX);
	const bool Result =
		agxTerrain::TerrainMaterialLibrary::loadMaterialProfile(MaterialNameAGX, Material);

	if (!Result)
		return {};

	return AGXBarrierFactories::CreateTerrainMaterialBarrier(Material);
}
