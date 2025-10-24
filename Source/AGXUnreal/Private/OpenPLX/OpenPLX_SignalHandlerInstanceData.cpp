// Copyright 2025, Algoryx Simulation AB.

#include "OpenPLX/OpenPLX_SignalHandlerInstanceData.h"

// AGX Dynamics for Unreal includes.
#include "OpenPLX/OpenPLX_SignalHandlerComponent.h"

FOpenPLX_SignalHandlerInstanceData::FOpenPLX_SignalHandlerInstanceData(
	const UOpenPLX_SignalHandlerComponent& Component)
	: FActorComponentInstanceData(&Component)
{
	NativeAddresses = Component.GetNativeAddresses();
}

void FOpenPLX_SignalHandlerInstanceData::ApplyToComponent(
	UActorComponent* Component, const ECacheApplyPhase CacheApplyPhase)
{
	FActorComponentInstanceData::ApplyToComponent(Component, CacheApplyPhase);
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
	return Super::ContainsData() || HasAddresses();
}

bool FOpenPLX_SignalHandlerInstanceData::HasAddresses() const
{
	// Simple check, we don't go through all addresses here. They should either all be set
	// or all be unset.
	return NativeAddresses.AssemblyAddress != 0 || NativeAddresses.ModelRegistryAddress != 0;
}
