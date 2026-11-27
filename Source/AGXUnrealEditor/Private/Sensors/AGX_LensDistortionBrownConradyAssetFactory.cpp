// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_LensDistortionBrownConradyAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_LensDistortionBrownConrady.h"

UAGX_LensDistortionBrownConradyAssetFactory::UAGX_LensDistortionBrownConradyAssetFactory(
	const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_LensDistortionBrownConrady::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_LensDistortionBrownConradyAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_LensDistortionBrownConrady::StaticClass()));
	return NewObject<UAGX_LensDistortionBrownConrady>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
