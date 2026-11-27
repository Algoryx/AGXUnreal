// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/AGX_CameraLensSingleElementAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/AGX_CameraLensSingleElement.h"

UAGX_CameraLensSingleElementAssetFactory::UAGX_CameraLensSingleElementAssetFactory(
	const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_CameraLensSingleElement::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_CameraLensSingleElementAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_CameraLensSingleElement::StaticClass()));
	return NewObject<UAGX_CameraLensSingleElement>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
