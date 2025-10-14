// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include <Contacts/AGX_ContactEnums.h>

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Templates/Function.h"

class FSimulationBarrier;
class FShapeContactBarrier;
struct FAnyShapeBarrier;

// TODO Create A Contact Event Listener Barrier class, return that from Create Contact Event
// Listener, and let AGX Simulation hold it.

void AGXUNREALBARRIER_API CreateContactEventListener(
	FSimulationBarrier& Simulation,
	TFunction<EAGX_KeepContactPolicy(double Time, FShapeContactBarrier&)> ImpactCallback,
	TFunction<EAGX_KeepContactPolicy(double Time, FShapeContactBarrier&)> ContactCallback,
	TFunction<void(double Time, FAnyShapeBarrier&, FAnyShapeBarrier&)> SeparationCallback);
