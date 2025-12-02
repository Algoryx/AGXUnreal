// Copyright 2025, Algoryx Simulation AB.

#include "Sensors/AGX_LidarLambertianOpaqueMaterialAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LidarLambertianOpaqueMaterial.h"

UAGX_LidarLambertianOpaqueMaterialAssetFactory::UAGX_LidarLambertianOpaqueMaterialAssetFactory(
	const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_LidarLambertianOpaqueMaterial::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_LidarLambertianOpaqueMaterialAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_LidarLambertianOpaqueMaterial::StaticClass()));
	return NewObject<UAGX_LidarLambertianOpaqueMaterial>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
