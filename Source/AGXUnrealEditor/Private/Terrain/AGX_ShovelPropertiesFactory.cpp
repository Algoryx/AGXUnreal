// Copyright 2025, Algoryx Simulation AB.

// AGX Dynamics for Unreal includes.
#include "Terrain/AGX_ShovelPropertiesFactory.h"

#include "Terrain/AGX_ShovelProperties.h"

UAGX_ShovelPropertiesFactory::UAGX_ShovelPropertiesFactory(const FObjectInitializer& Initializer)
	: Super(Initializer)
{
	SupportedClass = UAGX_ShovelProperties::StaticClass();

	// The operations this factory supports.
	bCreateNew = true;
	bEditorImport = false;

	bEditAfterNew = true;
}

UObject* UAGX_ShovelPropertiesFactory::FactoryCreateNew(
	UClass* Class, UObject* Parent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_ShovelProperties::StaticClass()));
	return NewObject<UAGX_ShovelProperties>(Parent, Class, Name, Flags | RF_Transactional, Context);
}
