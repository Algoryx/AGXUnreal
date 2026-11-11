// Copyright 2026, Algoryx Simulation AB.

#include "Distributed/AGX_DistributedSpawnAsync.h"
#include "Distributed/AGX_DistributedWorldSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UAGX_DistributedSpawnAsync* UAGX_DistributedSpawnAsync::SpawnDistributedEntity(
	UObject* WorldContextObject, FAGX_DistributedSpawnOptions Options)
{
	UAGX_DistributedSpawnAsync* Action = NewObject<UAGX_DistributedSpawnAsync>();
	Action->Context = WorldContextObject;
	Action->SpawnOptions = Options;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UAGX_DistributedSpawnAsync::Activate()
{
	UWorld* World = GEngine->GetWorldFromContextObject(Context, EGetWorldErrorMode::ReturnNull);
	UAGX_DistributedWorldSubsystem* Subsystem =
		World != nullptr ? World->GetSubsystem<UAGX_DistributedWorldSubsystem>() : nullptr;
	const FGuid Id = Subsystem != nullptr
						 ? Subsystem->QueueSpawn(
							   SpawnOptions, FAGX_DistributedSpawnCallback::CreateUObject(
												 this, &ThisClass::Completed))
						 : FGuid();
	Queued.Broadcast(Id.IsValid(), -1, nullptr, FString(), Id);
	if (!Id.IsValid())
	{
		Fail.Broadcast(
			false, -1, nullptr,
			TEXT(
				"Cannot queue spawn: check scenario, asset mapping, unit scale and world "
				"lifetime."),
			Id);
		SetReadyToDestroy();
	}
}

void UAGX_DistributedSpawnAsync::Completed(const FAGX_DistributedSpawnResult& Result)
{
	if (Result.State == EAGX_DistributedSpawnState::Succeeded)
		Success.Broadcast(true, Result.EntityId, Result.Entity, FString(), Result.RequestId);
	else
		Fail.Broadcast(true, Result.EntityId, nullptr, Result.ErrorMessage, Result.RequestId);
	SetReadyToDestroy();
}
