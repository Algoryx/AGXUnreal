// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_LivoxHorizonParametersAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LivoxHorizonParameters.h"

UAGX_LivoxHorizonParametersAssetFactory::UAGX_LivoxHorizonParametersAssetFactory(
	const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_LivoxHorizonParameters::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_LivoxHorizonParametersAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_LivoxHorizonParameters::StaticClass()));
	return NewObject<UAGX_LivoxHorizonParameters>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
