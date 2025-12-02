// Copyright 2025, Algoryx Simulation AB.

#include "Sensors/AGX_OusterOS1ParametersAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_OusterOS1Parameters.h"

UAGX_OusterOS1ParametersAssetFactory::UAGX_OusterOS1ParametersAssetFactory(const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_OusterOS1Parameters::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_OusterOS1ParametersAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_OusterOS1Parameters::StaticClass()));
	return NewObject<UAGX_OusterOS1Parameters>(InParent, Class, Name, Flags | RF_Transactional, Context);
}
