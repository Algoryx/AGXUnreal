// Copyright 2025, Algoryx Simulation AB.

#include "Materials/AGX_ShapeMaterialAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "Materials/AGX_ShapeMaterial.h"

UAGX_ShapeMaterialFactory::UAGX_ShapeMaterialFactory(const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_ShapeMaterial::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_ShapeMaterialFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_ShapeMaterial::StaticClass()));
	return NewObject<UAGX_ShapeMaterial>(InParent, Class, Name, Flags | RF_Transactional, Context);
}
