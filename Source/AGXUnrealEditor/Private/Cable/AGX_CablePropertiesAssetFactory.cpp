// Copyright 2025, Algoryx Simulation AB.

#include "Cable/AGX_CablePropertiesAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "Cable/AGX_CableProperties.h"

UAGX_CablePropertiesFactory::UAGX_CablePropertiesFactory(const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_CableProperties::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_CablePropertiesFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_CableProperties::StaticClass()));
	return NewObject<UAGX_CableProperties>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
