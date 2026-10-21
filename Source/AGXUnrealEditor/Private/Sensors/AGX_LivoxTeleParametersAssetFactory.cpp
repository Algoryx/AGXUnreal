// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_LivoxTeleParametersAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LivoxTeleParameters.h"

UAGX_LivoxTeleParametersAssetFactory::UAGX_LivoxTeleParametersAssetFactory(
	const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_LivoxTeleParameters::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_LivoxTeleParametersAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_LivoxTeleParameters::StaticClass()));
	return NewObject<UAGX_LivoxTeleParameters>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
