// Copyright 2026, Algoryx Simulation AB.

#include "Vehicle/AGX_DavisSteeringParametersAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "Vehicle/AGX_DavisSteeringParameters.h"

UAGX_DavisSteeringParametersAssetFactory::UAGX_DavisSteeringParametersAssetFactory(
	const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_DavisSteeringParameters::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_DavisSteeringParametersAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_DavisSteeringParameters::StaticClass()));
	return NewObject<UAGX_DavisSteeringParameters>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
