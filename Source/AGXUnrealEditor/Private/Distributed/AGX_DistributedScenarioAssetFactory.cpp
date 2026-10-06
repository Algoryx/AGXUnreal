// Copyright 2026, Algoryx Simulation AB.

#include "Distributed/AGX_DistributedScenarioAssetFactory.h"

#include "Distributed/AGX_DistributedScenarioAsset.h"

UAGX_DistributedScenarioAssetFactory::UAGX_DistributedScenarioAssetFactory(
	const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SupportedClass = UAGX_DistributedScenarioAsset::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAGX_DistributedScenarioAssetFactory::FactoryCreateNew(
	UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context,
	FFeedbackContext* Warn)
{
	check(Class->IsChildOf(UAGX_DistributedScenarioAsset::StaticClass()));
	return NewObject<UAGX_DistributedScenarioAsset>(
		InParent, Class, Name, Flags | RF_Transactional, Context);
}
