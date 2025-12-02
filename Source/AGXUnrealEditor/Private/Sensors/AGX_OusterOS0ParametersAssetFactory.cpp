// Copyright 2025, Algoryx Simulation AB.

#include "Sensors/AGX_OusterOS0ParametersAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_OusterOS0Parameters.h"

UAGX_OusterOS0ParametersAssetFactory::UAGX_OusterOS0ParametersAssetFactory(const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_OusterOS0Parameters::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_OusterOS0ParametersAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_OusterOS0Parameters::StaticClass()));
	return NewObject<UAGX_OusterOS0Parameters>(InParent, Class, Name, Flags | RF_Transactional, Context);
}
