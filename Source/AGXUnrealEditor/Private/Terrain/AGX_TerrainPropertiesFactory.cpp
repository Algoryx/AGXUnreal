// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainPropertiesFactory.h"

// AGX Dynamics for Unreal includes.
#include "Terrain/AGX_TerrainProperties.h"

UAGX_TerrainPropertiesFactory::UAGX_TerrainPropertiesFactory(const FObjectInitializer& Initializer)
	: Super(Initializer)
{
	SupportedClass = UAGX_TerrainProperties::StaticClass();

	// The operations this factory supports.
	bCreateNew = true;
	bEditorImport = false;

	bEditAfterNew = true;
}

UObject* UAGX_TerrainPropertiesFactory::FactoryCreateNew(
	UClass* Class, UObject* Parent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_TerrainProperties::StaticClass()));
	return NewObject<UAGX_TerrainProperties>(Parent, Class, Name, Flags | RF_Transactional, Context);
}
