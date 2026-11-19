// Copyright 2026, Algoryx Simulation AB.

#include "Distributed/OpenPLX_DistributedSignalRerouter.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Distributed/AGX_DistributedWorldSubsystem.h"

// Unreal Engine includes.
#include "Engine/World.h"

FOpenPLX_DistributedSignalRerouter::FOpenPLX_DistributedSignalRerouter(int64 InEntityId)
	: EntityId(InEntityId)
{
}

bool FOpenPLX_DistributedSignalRerouter::SendReal(
	UWorld* World, const FOpenPLX_Input& Input, double Value)
{
	UAGX_DistributedWorldSubsystem* Subsystem =
		World != nullptr ? World->GetSubsystem<UAGX_DistributedWorldSubsystem>() : nullptr;
	if (Subsystem == nullptr || !Subsystem->IsPossessed(EntityId))
		return false;
	return Subsystem->RouteSignal(EntityId, Input.Name.ToString(), Value);
}

bool FOpenPLX_DistributedSignalRerouter::SendRangeReal(UWorld*, const FOpenPLX_Input&, FVector2D)
{
	UE_LOG(
		LogAGX, Warning,
		TEXT("FOpenPLX_DistributedSignalRerouter::SendRangeReal is not implemented."));
	return false;
}

bool FOpenPLX_DistributedSignalRerouter::SendVector2(UWorld*, const FOpenPLX_Input&, FVector2D)
{
	UE_LOG(
		LogAGX, Warning,
		TEXT("FOpenPLX_DistributedSignalRerouter::SendVector2 is not implemented."));
	return false;
}

bool FOpenPLX_DistributedSignalRerouter::SendVector(UWorld*, const FOpenPLX_Input&, FVector)
{
	UE_LOG(
		LogAGX, Warning,
		TEXT("FOpenPLX_DistributedSignalRerouter::SendVector is not implemented."));
	return false;
}

bool FOpenPLX_DistributedSignalRerouter::SendInteger(UWorld*, const FOpenPLX_Input&, int64)
{
	UE_LOG(
		LogAGX, Warning,
		TEXT("FOpenPLX_DistributedSignalRerouter::SendInteger is not implemented."));
	return false;
}

bool FOpenPLX_DistributedSignalRerouter::SendBoolean(UWorld*, const FOpenPLX_Input&, bool)
{
	UE_LOG(
		LogAGX, Warning,
		TEXT("FOpenPLX_DistributedSignalRerouter::SendBoolean is not implemented."));
	return false;
}
