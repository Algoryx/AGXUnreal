// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Factories/Factory.h"

#include "AGX_TerrainWheelSettingsFactory.generated.h"

class FObjectInitializer;

/**
 * Asset Factory for UAGX_TerrainWheelSettings, making it possible to create asset objects in the
 * Editor.
 */
UCLASS()
class AGXUNREALEDITOR_API UAGX_TerrainWheelSettingsFactory : public UFactory
{
	GENERATED_BODY()

public:
	UAGX_TerrainWheelSettingsFactory(const class FObjectInitializer& OBJ);

	// ~Begin UFactory interface.
	virtual UObject* FactoryCreateNew(
		UClass* Class, UObject* Parent, FName Name, EObjectFlags Flags, UObject* Context,
		FFeedbackContext* Warn) override;
	// ~End UFactory interface.
};
