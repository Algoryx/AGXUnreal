// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainWheelDeformationPropertiesFactory.h"

// AGX Dynamics for Unreal includes.
#include "Terrain/AGX_TerrainWheelDeformationProperties.h"

UAGX_TerrainWheelDeformationPropertiesFactory::UAGX_TerrainWheelDeformationPropertiesFactory(
	const FObjectInitializer& Initializer)
	: Super(Initializer)
{
	SupportedClass = UAGX_TerrainWheelDeformationProperties::StaticClass();

	// The operations this factory supports.
	bCreateNew = true;
	bEditorImport = false;

	bEditAfterNew = true;
}

UObject* UAGX_TerrainWheelDeformationPropertiesFactory::FactoryCreateNew(
	UClass* Class, UObject* Parent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_TerrainWheelDeformationProperties::StaticClass()));
	return NewObject<UAGX_TerrainWheelDeformationProperties>(
		Parent, Class, Name, Flags | RF_Transactional, Context);
}
