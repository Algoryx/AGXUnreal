// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "AGX_ShapeMaterialAssetFactory.generated.h"

/**
 * Asset Factory for UAGX_ShapeMaterial, making it possible to create asset objects in the
 * Editor.
 */
UCLASS()
class AGXUNREALEDITOR_API UAGX_ShapeMaterialFactory : public UFactory
{
	GENERATED_BODY()

public:
	UAGX_ShapeMaterialFactory(const class FObjectInitializer& OBJ);

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
