// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_LivoxMid70ParametersAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LivoxMid70Parameters.h"

UAGX_LivoxMid70ParametersAssetFactory::UAGX_LivoxMid70ParametersAssetFactory(
	const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_LivoxMid70Parameters::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_LivoxMid70ParametersAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_LivoxMid70Parameters::StaticClass()));
	return NewObject<UAGX_LivoxMid70Parameters>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
