// Copyright 2026, Algoryx Simulation AB.

#include "Distributed/OpenPLX_DistributedSignalRerouter.h"

// AGX Dynamics for Unreal includes.
#include "Distributed/AGX_DistributedWorldSubsystem.h"

// Unreal Engine includes.
#include "Engine/World.h"

namespace
{
	bool RouteSignal(
		UWorld* World, int64 EntityId, const FOpenPLX_Input& Input,
		const FAGXDistributedSignalValue& Value)
	{
		UAGX_DistributedWorldSubsystem* Subsystem =
			World != nullptr ? World->GetSubsystem<UAGX_DistributedWorldSubsystem>() : nullptr;
		return Subsystem != nullptr && Subsystem->IsPossessed(EntityId) &&
			   Subsystem->RouteSignalValue(EntityId, Input, Value);
	}
}

FOpenPLX_DistributedSignalRerouter::FOpenPLX_DistributedSignalRerouter(int64 InEntityId)
	: EntityId(InEntityId)
{
}

bool FOpenPLX_DistributedSignalRerouter::SendReal(
	UWorld* World, const FOpenPLX_Input& Input, double Value)
{
	return RouteSignal(World, EntityId, Input, Value);
}

bool FOpenPLX_DistributedSignalRerouter::SendRangeReal(
	UWorld* World, const FOpenPLX_Input& Input, FVector2D Value)
{
	return RouteSignal(World, EntityId, Input, Value);
}

bool FOpenPLX_DistributedSignalRerouter::SendVector2(
	UWorld* World, const FOpenPLX_Input& Input, FVector2D Value)
{
	return RouteSignal(World, EntityId, Input, Value);
}

bool FOpenPLX_DistributedSignalRerouter::SendVector(
	UWorld* World, const FOpenPLX_Input& Input, FVector Value)
{
	return RouteSignal(World, EntityId, Input, Value);
}

bool FOpenPLX_DistributedSignalRerouter::SendInteger(
	UWorld* World, const FOpenPLX_Input& Input, int64 Value)
{
	return RouteSignal(World, EntityId, Input, Value);
}

bool FOpenPLX_DistributedSignalRerouter::SendBoolean(
	UWorld* World, const FOpenPLX_Input& Input, bool Value)
{
	return RouteSignal(World, EntityId, Input, Value);
}
