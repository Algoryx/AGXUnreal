// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/AGX_CustomRayPatternParametersAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_CustomRayPatternParameters.h"

UAGX_CustomRayPatternParametersAssetFactory::UAGX_CustomRayPatternParametersAssetFactory(const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_CustomRayPatternParameters::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_CustomRayPatternParametersAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_CustomRayPatternParameters::StaticClass()));
	return NewObject<UAGX_CustomRayPatternParameters>(InParent, Class, Name, Flags | RF_Transactional, Context);
}
