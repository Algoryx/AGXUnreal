// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "OpenPLX/OpenPLX_SignalRerouter.h"

/** Routes outgoing OpenPLX signals for one distributed entity. */
class AGXUNREAL_API FOpenPLX_DistributedSignalRerouter : public FOpenPLX_SignalRerouter
{
public:
	explicit FOpenPLX_DistributedSignalRerouter(int64 InEntityId);

	/** Forward the input name and value unchanged. Unit conversion is not implemented yet. */
	virtual bool SendReal(UWorld* World, const FOpenPLX_Input& Input, double Value) override;

	virtual bool SendRangeReal(
		UWorld* World, const FOpenPLX_Input& Input, FVector2D Value) override;
	virtual bool SendVector2(UWorld* World, const FOpenPLX_Input& Input, FVector2D Value) override;
	virtual bool SendVector(UWorld* World, const FOpenPLX_Input& Input, FVector Value) override;
	virtual bool SendInteger(UWorld* World, const FOpenPLX_Input& Input, int64 Value) override;
	virtual bool SendBoolean(UWorld* World, const FOpenPLX_Input& Input, bool Value) override;

private:
	const int64 EntityId;
};
