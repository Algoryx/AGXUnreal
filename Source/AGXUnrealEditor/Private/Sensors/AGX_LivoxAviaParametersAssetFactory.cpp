// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_LivoxAviaParametersAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LivoxAviaParameters.h"

UAGX_LivoxAviaParametersAssetFactory::UAGX_LivoxAviaParametersAssetFactory(
	const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_LivoxAviaParameters::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_LivoxAviaParametersAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_LivoxAviaParameters::StaticClass()));
	return NewObject<UAGX_LivoxAviaParameters>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
