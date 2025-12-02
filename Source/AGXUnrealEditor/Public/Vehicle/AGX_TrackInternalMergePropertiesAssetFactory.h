// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Factories/Factory.h"

#include "AGX_TrackInternalMergePropertiesAssetFactory.generated.h"

/**
 * Asset Factory for UAGX_TrackInternalMergeProperties, making it possible to create asset
 * objects in the Editor.
 */
UCLASS()
class AGXUNREALEDITOR_API UAGX_TrackInternalMergePropertiesAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UAGX_TrackInternalMergePropertiesAssetFactory(const class FObjectInitializer& OBJ);

	virtual UObject* FactoryCreateNew(
		UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
		FFeedbackContext* Warn) override;

protected:
	virtual bool IsMacroFactory() const
	{
		return false;
	}
};
