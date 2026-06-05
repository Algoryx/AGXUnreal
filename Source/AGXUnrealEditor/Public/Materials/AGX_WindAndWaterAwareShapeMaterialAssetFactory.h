// Copyright 2023, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "AGX_WindAndWaterAwareShapeMaterialAssetFactory.generated.h"

/**
 * Asset Factory for UAGX_WindAndWaterAwareShapeMaterial, making it possible to create asset objects in the
 * Editor.
 */
UCLASS()
class AGXUNREALEDITOR_API UAGX_WindAndWaterAwareShapeMaterialFactory : public UFactory
{
	GENERATED_BODY()

public:
	UAGX_WindAndWaterAwareShapeMaterialFactory(const class FObjectInitializer& OBJ);

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
