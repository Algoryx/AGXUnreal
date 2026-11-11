// Copyright 2026, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"

#include "AGX_DistributedAssetFactory.generated.h"

/** Creates AGX Distributed asset descriptors from the Content Browser. */
UCLASS()
class AGXUNREALEDITOR_API UAGX_DistributedAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UAGX_DistributedAssetFactory(const FObjectInitializer& ObjectInitializer);

	virtual UObject* FactoryCreateNew(
		UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
		FFeedbackContext* Warn) override;
};
