// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainWheelMaterialAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "Terrain/AGX_TerrainWheelMaterial.h"

UAGX_TerrainWheelMaterialAssetFactory::UAGX_TerrainWheelMaterialAssetFactory(
	const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_TerrainWheelMaterial::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_TerrainWheelMaterialAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_TerrainWheelMaterial::StaticClass()));
	return NewObject<UAGX_TerrainWheelMaterial>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
