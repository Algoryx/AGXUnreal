// Copyright 2025, Algoryx Simulation AB.

#include "Wire/AGX_WireActor.h"

// AGX Dynamics for Unreal include.s
#include "Wire/AGX_WireComponent.h"

AAGX_WireActor::AAGX_WireActor()
{
	PrimaryActorTick.bCanEverTick = false;

	WireComponent = CreateDefaultSubobject<UAGX_WireComponent>(TEXT("WireComponent"));
	SetRootComponent(WireComponent);
}
