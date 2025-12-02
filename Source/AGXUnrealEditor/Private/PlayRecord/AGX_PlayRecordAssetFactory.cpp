// Copyright 2025, Algoryx Simulation AB.

#include "PlayRecord/AGX_PlayRecordAssetFactory.h"

// AGX Dynamics for Unreal includes.
#include "PlayRecord/AGX_PlayRecord.h"

UAGX_PlayRecordAssetFactory::UAGX_PlayRecordAssetFactory(const class FObjectInitializer& OBJ)
	: Super(OBJ)
{
	SupportedClass = UAGX_PlayRecord::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_PlayRecordAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_PlayRecord::StaticClass()));
	return NewObject<UAGX_PlayRecord>(InParent, Class, Name, Flags | RF_Transactional, Context);
}
