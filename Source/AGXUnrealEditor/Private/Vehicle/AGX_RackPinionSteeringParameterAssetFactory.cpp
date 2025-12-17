// Copyright 2025, Algoryx Simulation AB.

#include "Vehicle/AGX_RackPinionSteeringParametersAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "Vehicle/AGX_RackPinionSteeringParameters.h"

UAGX_RackPinionSteeringParametersAssetFactory::UAGX_RackPinionSteeringParametersAssetFactory(
	const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_RackPinionSteeringParameters::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_RackPinionSteeringParametersAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_RackPinionSteeringParameters::StaticClass()));
	return NewObject<UAGX_RackPinionSteeringParameters>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
