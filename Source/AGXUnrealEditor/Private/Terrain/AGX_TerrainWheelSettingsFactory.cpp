// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainWheelSettingsFactory.h"

// AGX Dynamics for Unreal includes.
#include "Terrain/AGX_TerrainWheelSettings.h"

UAGX_TerrainWheelSettingsFactory::UAGX_TerrainWheelSettingsFactory(
	const FObjectInitializer& Initializer)
	: Super(Initializer)
{
	SupportedClass = UAGX_TerrainWheelSettings::StaticClass();

	// The operations this factory supports.
	bCreateNew = true;
	bEditorImport = false;

	bEditAfterNew = true;
}

UObject* UAGX_TerrainWheelSettingsFactory::FactoryCreateNew(
	UClass* Class, UObject* Parent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_TerrainWheelSettings::StaticClass()));
	return NewObject<UAGX_TerrainWheelSettings>(
		Parent, Class, Name, Flags | RF_Transactional, Context);
}
