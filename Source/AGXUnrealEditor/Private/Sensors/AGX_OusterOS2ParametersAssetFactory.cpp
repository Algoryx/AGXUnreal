// Copyright 2025, Algoryx Simulation AB.

#include "Sensors/AGX_OusterOS2ParametersAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_OusterOS2Parameters.h"

UAGX_OusterOS2ParametersAssetFactory::UAGX_OusterOS2ParametersAssetFactory(const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_OusterOS2Parameters::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_OusterOS2ParametersAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_OusterOS2Parameters::StaticClass()));
	return NewObject<UAGX_OusterOS2Parameters>(InParent, Class, Name, Flags | RF_Transactional, Context);
}
