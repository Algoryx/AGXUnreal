// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"

#include "AGX_ConstraintMergeSplitThresholdsAssetFactory.generated.h"

UCLASS()
class AGXUNREALEDITOR_API UAGX_ConstraintMergeSplitThresholdsAssetFactory : public UFactory
{
	GENERATED_BODY()

public:
	UAGX_ConstraintMergeSplitThresholdsAssetFactory(const class FObjectInitializer& OBJ);

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
