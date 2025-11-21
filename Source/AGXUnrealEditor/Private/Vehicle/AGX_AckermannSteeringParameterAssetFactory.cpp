// Copyright 2025, Algoryx Simulation AB.

#include "Vehicle/AGX_AckermannSteeringParametersAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "Vehicle/AGX_AckermannSteeringParameters.h"

UAGX_AckermannSteeringParametersAssetFactory::UAGX_AckermannSteeringParametersAssetFactory(
	const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_AckermannSteeringParameters::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_AckermannSteeringParametersAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_AckermannSteeringParameters::StaticClass()));
	return NewObject<UAGX_AckermannSteeringParameters>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
