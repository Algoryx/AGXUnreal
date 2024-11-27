// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_LidarAmbientMaterialAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarAmbientMaterial.h"

UAGX_LidarAmbientMaterialAssetFactory::UAGX_LidarAmbientMaterialAssetFactory(
	const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_LidarAmbientMaterial::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_LidarAmbientMaterialAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_LidarAmbientMaterial::StaticClass()));
	return NewObject<UAGX_LidarAmbientMaterial>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
