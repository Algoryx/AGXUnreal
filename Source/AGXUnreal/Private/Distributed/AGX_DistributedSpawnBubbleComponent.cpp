// Copyright 2026, Algoryx Simulation AB.
#include "Distributed/AGX_DistributedSpawnBubbleComponent.h"
#include "Distributed/AGX_DistributedWorldSubsystem.h"
#include "Engine/World.h"
void UAGX_DistributedSpawnBubbleComponent::BeginPlay()
{
	Super::BeginPlay();
	if (auto* Subsystem = GetWorld()->GetSubsystem<UAGX_DistributedWorldSubsystem>())
		Subsystem->RegisterSpawnBubble(this);
}
void UAGX_DistributedSpawnBubbleComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	if (auto* Subsystem = GetWorld()->GetSubsystem<UAGX_DistributedWorldSubsystem>())
		Subsystem->UnregisterSpawnBubble(this);
	Super::EndPlay(Reason);
}
