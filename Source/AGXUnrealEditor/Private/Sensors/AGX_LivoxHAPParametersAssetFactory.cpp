// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_LivoxHAPParametersAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LivoxHAPParameters.h"

UAGX_LivoxHAPParametersAssetFactory::UAGX_LivoxHAPParametersAssetFactory(
	const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_LivoxHAPParameters::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_LivoxHAPParametersAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_LivoxHAPParameters::StaticClass()));
	return NewObject<UAGX_LivoxHAPParameters>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
