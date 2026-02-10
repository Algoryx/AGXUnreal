// Copyright 2026, Algoryx Simulation AB.

#include "Vehicle/AGX_BellCrankSteeringParametersAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "Vehicle/AGX_BellCrankSteeringParameters.h"

UAGX_BellCrankSteeringParametersAssetFactory::UAGX_BellCrankSteeringParametersAssetFactory(
	const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_BellCrankSteeringParameters::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_BellCrankSteeringParametersAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_BellCrankSteeringParameters::StaticClass()));
	return NewObject<UAGX_BellCrankSteeringParameters>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
