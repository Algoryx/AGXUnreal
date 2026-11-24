// Copyright 2026, Algoryx Simulation AB.

#include "Distributed/AGX_DistributedSimulationAuthorityAsync.h"

// AGX Dynamics for Unreal includes.
#include "Distributed/AGX_DistributedWorldSubsystem.h"

// Unreal Engine includes.
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UAGX_DistributedSimulationAuthorityAsync*
UAGX_DistributedSimulationAuthorityAsync::RequestDistributedSimulationAuthority(
	AActor* TargetActor, bool bSimulate)
{
	auto* Action = NewObject<UAGX_DistributedSimulationAuthorityAsync>();
	Action->Entity = TargetActor;
	Action->bRequestedSimulationAuthority = bSimulate;
	if (IsValid(TargetActor))
		Action->RegisterWithGameInstance(TargetActor);
	return Action;
}

void UAGX_DistributedSimulationAuthorityAsync::Activate()
{
	AActor* Actor = Entity.Get();
	UWorld* World = Actor != nullptr ? Actor->GetWorld() : nullptr;
	auto* Subsystem =
		World != nullptr ? World->GetSubsystem<UAGX_DistributedWorldSubsystem>() : nullptr;
	const FGuid Id = Subsystem != nullptr
						 ? Subsystem->RequestActorSimulationAuthority(
							   Actor, bRequestedSimulationAuthority,
							   FAGX_DistributedSimulationAuthorityCallback::CreateUObject(
								   this, &ThisClass::Completed))
						 : FGuid();
	if (!Id.IsValid())
	{
		Fail.Broadcast(
			Actor, TEXT(
					   "Cannot request simulation authority: check the distributed target, "
					   "connection and pending requests."));
		SetReadyToDestroy();
	}
}

void UAGX_DistributedSimulationAuthorityAsync::Completed(
	bool bAccepted, AActor* CompletedEntity, const FString& ErrorMessage)
{
	if (bAccepted)
		Success.Broadcast(CompletedEntity, FString());
	else
		Fail.Broadcast(CompletedEntity, ErrorMessage);
	SetReadyToDestroy();
}
