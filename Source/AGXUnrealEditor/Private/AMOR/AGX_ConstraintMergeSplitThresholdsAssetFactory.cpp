// Copyright 2025, Algoryx Simulation AB.

#include "AMOR/AGX_ConstraintMergeSplitThresholdsAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "AMOR/AGX_ConstraintMergeSplitThresholds.h"

UAGX_ConstraintMergeSplitThresholdsAssetFactory::UAGX_ConstraintMergeSplitThresholdsAssetFactory(
	const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_ConstraintMergeSplitThresholds::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_ConstraintMergeSplitThresholdsAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_ConstraintMergeSplitThresholds::StaticClass()));
	return NewObject<UAGX_ConstraintMergeSplitThresholds>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
