// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Class.h"

#include "OpenPLX_SignalHandlerNativeAddresses.generated.h"

/**
 * Holds the addresses of all native objects referenced by the FOpenPLXSignalHandler class.
 * Used for storing instance data of UOpenPLX_SignalHandlerComponent during Blueprint
 * Reconstruction.
 */
USTRUCT()
struct AGXCOMMON_API FOpenPLX_SignalHandlerNativeAddresses
{
	GENERATED_BODY()

	bool operator==(const FOpenPLX_SignalHandlerNativeAddresses& Other) const = default;

	uint64 AssemblyAddress {0};
	uint64 InputSignalListenerAddress {0};
	uint64 OutputSignalListenerAddress {0};
	uint64 ModelRegistryAddress {0};
	int32 ModelHandle {-1};
};
