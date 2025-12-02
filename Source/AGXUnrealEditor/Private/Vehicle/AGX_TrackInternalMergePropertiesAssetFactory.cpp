// Copyright 2025, Algoryx Simulation AB.

#include "Vehicle/AGX_TrackInternalMergePropertiesAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "Vehicle/AGX_TrackInternalMergeProperties.h"

UAGX_TrackInternalMergePropertiesAssetFactory::UAGX_TrackInternalMergePropertiesAssetFactory(
	const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_TrackInternalMergeProperties::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_TrackInternalMergePropertiesAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_TrackInternalMergeProperties::StaticClass()));
	return NewObject<UAGX_TrackInternalMergeProperties>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
