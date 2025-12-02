// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Factories/Factory.h"

#include "AGX_ShovelPropertiesFactory.generated.h"

class FObjectInitializer;

UCLASS()
class AGXUNREALEDITOR_API UAGX_ShovelPropertiesFactory : public UFactory
{
	GENERATED_BODY()

public:
	UAGX_ShovelPropertiesFactory(const class FObjectInitializer& OBJ);

	// ~Begin UFactory interface.
	virtual UObject* FactoryCreateNew(
		UClass* Class, UObject* Parent, FName Name, EObjectFlags Flags, UObject* Context,
		FFeedbackContext* Warn) override;
	// ~End UFactory interface.
};
