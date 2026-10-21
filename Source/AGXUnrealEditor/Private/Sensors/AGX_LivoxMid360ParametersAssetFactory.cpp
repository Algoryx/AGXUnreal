// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_LivoxMid360ParametersAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LivoxMid360Parameters.h"

UAGX_LivoxMid360ParametersAssetFactory::UAGX_LivoxMid360ParametersAssetFactory(
	const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_LivoxMid360Parameters::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_LivoxMid360ParametersAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_LivoxMid360Parameters::StaticClass()));
	return NewObject<UAGX_LivoxMid360Parameters>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
