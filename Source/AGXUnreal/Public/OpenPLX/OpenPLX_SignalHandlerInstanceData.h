// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Components/ActorComponent.h"
#include "OpenPLX/OpenPLX_SignalHandlerNativeAddresses.h"

#include "OpenPLX_SignalHandlerInstanceData.generated.h"

class UOpenPLX_SignalHandlerComponent;

/**
 * Component Instance Data for the OpenPLX Signal Handler Component. Will store the address of the
 * any native object referenced by the FOpenPLXSignalHandler barrier class.
 */
USTRUCT()
struct AGXUNREAL_API FOpenPLX_SignalHandlerInstanceData : public FActorComponentInstanceData
{
	GENERATED_BODY();

	FOpenPLX_SignalHandlerInstanceData() = default;
	FOpenPLX_SignalHandlerInstanceData(const UOpenPLX_SignalHandlerComponent& Component);
	virtual ~FOpenPLX_SignalHandlerInstanceData() = default;

	//~ Begin FComponentInstanceData interface.
	virtual void ApplyToComponent(
		UActorComponent* Component, const ECacheApplyPhase CacheApplyPhase) override;

	virtual bool ContainsData() const override;
	//~ End FComponentInstanceData interface.

	bool HasAddresses() const;

private:
	UPROPERTY()
	FOpenPLX_SignalHandlerNativeAddresses NativeAddresses;
};
