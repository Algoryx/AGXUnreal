// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "AGX_TerrainMaterialAssetFactory.generated.h"

/**
 * Asset Factory for UAGX_TerrainMaterialAsset, making it possible to create asset objects in the
 * Editor.
 */
UCLASS()
class AGXUNREALEDITOR_API UAGX_TerrainMaterialAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UAGX_TerrainMaterialAssetFactory(const class FObjectInitializer& OBJ);

protected:
	virtual bool IsMacroFactory() const
	{
		return false;
	}

public:
	virtual UObject* FactoryCreateNew(
		UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
		FFeedbackContext* Warn) override;
};
