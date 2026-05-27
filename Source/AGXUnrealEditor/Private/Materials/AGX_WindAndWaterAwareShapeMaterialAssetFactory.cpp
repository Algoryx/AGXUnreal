// Copyright 2023, Algoryx Simulation AB.

#include "Materials/AGX_WindAndWaterAwareShapeMaterialAssetFactory.h"

// AGX Dynamics for Unreal includes.

#include "Model/AGX_WindAndWaterAwareShapeMaterial.h"

UAGX_WindAndWaterAwareShapeMaterialFactory::UAGX_WindAndWaterAwareShapeMaterialFactory(const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_WindAndWaterAwareShapeMaterial::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_WindAndWaterAwareShapeMaterialFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_WindAndWaterAwareShapeMaterial::StaticClass()));
	return NewObject<UAGX_WindAndWaterAwareShapeMaterial>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
