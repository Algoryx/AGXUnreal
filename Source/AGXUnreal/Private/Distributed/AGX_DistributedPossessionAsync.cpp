// Copyright 2026, Algoryx Simulation AB.

#include "Distributed/AGX_DistributedPossessionAsync.h"

// AGX Dynamics for Unreal includes.
#include "Distributed/AGX_DistributedWorldSubsystem.h"

// Unreal Engine includes.
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UAGX_DistributedPossessionAsync* UAGX_DistributedPossessionAsync::RequestDistributedPossession(
	AActor* TargetActor, bool bPossess)
{
	auto* Action = NewObject<UAGX_DistributedPossessionAsync>();
	Action->Entity = TargetActor;
	Action->bRequestedPossession = bPossess;
	if (IsValid(TargetActor))
		Action->RegisterWithGameInstance(TargetActor);
	return Action;
}

void UAGX_DistributedPossessionAsync::Activate()
{
	AActor* Actor = Entity.Get();
	UWorld* World = Actor != nullptr ? Actor->GetWorld() : nullptr;
	auto* Subsystem =
		World != nullptr ? World->GetSubsystem<UAGX_DistributedWorldSubsystem>() : nullptr;
	const FGuid Id =
		Subsystem != nullptr
			? Subsystem->RequestActorPossession(
				  Actor, bRequestedPossession,
				  FAGX_DistributedPossessionCallback::CreateUObject(this, &ThisClass::Completed))
			: FGuid();
	if (!Id.IsValid())
	{
		Fail.Broadcast(
			Actor, TEXT(
					   "Cannot request possession: check the distributed target, connection, "
					   "ownership and pending requests."));
		SetReadyToDestroy();
	}
}

void UAGX_DistributedPossessionAsync::Completed(
	bool bAccepted, AActor* CompletedEntity, const FString& ErrorMessage)
{
	if (bAccepted)
		Success.Broadcast(CompletedEntity, FString());
	else
		Fail.Broadcast(CompletedEntity, ErrorMessage);
	SetReadyToDestroy();
}
