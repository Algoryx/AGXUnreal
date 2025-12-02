// Copyright 2025, Algoryx Simulation AB.

#include "AMOR/AGX_ShapeContactMergeSplitThresholdsAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "AMOR/AGX_ShapeContactMergeSplitThresholds.h"

UAGX_ShapeContactMergeSplitThresholdsAssetFactory::
	UAGX_ShapeContactMergeSplitThresholdsAssetFactory(const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_ShapeContactMergeSplitThresholds::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_ShapeContactMergeSplitThresholdsAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_ShapeContactMergeSplitThresholds::StaticClass()));
	return NewObject<UAGX_ShapeContactMergeSplitThresholds>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
