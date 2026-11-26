// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Components/ActorComponent.h"

#include "AGX_DistributedSpawnPointVisualizerComponent.generated.h"

/** Editor-only visualization hook for a distributed spawn point's proximity sphere. */
UCLASS(ClassGroup = "AGX")
class AGXUNREAL_API UAGX_DistributedSpawnPointVisualizerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAGX_DistributedSpawnPointVisualizerComponent();
};
