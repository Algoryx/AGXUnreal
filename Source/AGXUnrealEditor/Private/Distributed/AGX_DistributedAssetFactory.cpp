// Copyright 2026, Algoryx Simulation AB.

#include "Distributed/AGX_DistributedAssetFactory.h"

#include "Distributed/AGX_DistributedAsset.h"

UAGX_DistributedAssetFactory::UAGX_DistributedAssetFactory(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UAGX_DistributedAsset::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_DistributedAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_DistributedAsset::StaticClass()));
	return NewObject<UAGX_DistributedAsset>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
