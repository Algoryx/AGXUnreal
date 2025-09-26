// Copyright 2025, Algoryx Simulation AB.

#include "OpenPLX/OpenPLX_SignalHandlerInstanceData.h"

// AGX Dynamics for Unreal includes.
#include "OpenPLX/OpenPLX_SignalHandlerComponent.h"

void FOpenPLX_SignalHandlerInstanceData::ApplyToComponent(
	UActorComponent* Component, const ECacheApplyPhase CacheApplyPhase)
{
	auto SignalHandlerComp = Cast<UOpenPLX_SignalHandlerComponent>(Component);
	if (SignalHandlerComp == nullptr)
		return;

	// Unreal Engine calls ApplyToComponent twice, so detect that and do nothing the second time.
	// But be aware that the first call happens before deserialization and the latter happens after,
	// so if you need the actual Property values then wait for the second call.
	if (SignalHandlerComp->GetNativeAddresses() == NativeAddresses)
	{
		return;
	}

	SignalHandlerComp->SetNativeAddresses(NativeAddresses);
}

bool FOpenPLX_SignalHandlerInstanceData::ContainsData() const
{
	return false;
}

void FOpenPLX_SignalHandlerInstanceData::AddReferencedObjects(FReferenceCollector& Collector)
{
}

void FOpenPLX_SignalHandlerInstanceData::FindAndReplaceInstances(
	const TMap<UObject*, UObject*>& OldToNewInstanceMap)
{
}

bool FOpenPLX_SignalHandlerInstanceData::HasAddresses() const
{
	return false;
}
