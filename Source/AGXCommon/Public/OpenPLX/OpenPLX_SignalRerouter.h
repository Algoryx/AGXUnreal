// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "OpenPLX/OpenPLX_Inputs.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

class UWorld;

/**
 * Base class for rerouting outgoing OpenPLX signals to another destination.
 *
 * Inputs are resolved descriptors. Values use the same Unreal units and coordinate conventions
 * as UOpenPLX_SignalHandlerComponent. Implementations handle any destination-specific conversion.
 * Each Send function returns true if the signal was accepted for rerouting, false otherwise.
 */
class AGXCOMMON_API FOpenPLX_SignalRerouter
{
public:
	FOpenPLX_SignalRerouter();
	FOpenPLX_SignalRerouter(const FOpenPLX_SignalRerouter& Other);
	FOpenPLX_SignalRerouter& operator=(const FOpenPLX_SignalRerouter& Other);
	virtual ~FOpenPLX_SignalRerouter();

	/** Reroute a Real signal. */
	virtual bool SendReal(UWorld* World, const FOpenPLX_Input& Input, double Value) = 0;

	/** Reroute a Real Range signal, with lower and upper bounds in X and Y. */
	virtual bool SendRangeReal(UWorld* World, const FOpenPLX_Input& Input, FVector2D Value) = 0;

	/** Reroute a Vector2 signal. */
	virtual bool SendVector2(UWorld* World, const FOpenPLX_Input& Input, FVector2D Value) = 0;

	/** Reroute a Vector signal. */
	virtual bool SendVector(UWorld* World, const FOpenPLX_Input& Input, FVector Value) = 0;

	/** Reroute an Integer signal. */
	virtual bool SendInteger(UWorld* World, const FOpenPLX_Input& Input, int64 Value) = 0;

	/** Reroute a Boolean signal. */
	virtual bool SendBoolean(UWorld* World, const FOpenPLX_Input& Input, bool Value) = 0;
};
