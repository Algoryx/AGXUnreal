// Copyright 2025, Algoryx Simulation AB.

#include "Contacts/ContactListenerBarrier.h"

// Contact Listener includes.
#include "Contacts/ContactEventListener.h"

// Unreal Engine includes.
#include "Modules/ModuleManager.h"

void CreateContactEventListener(
	FSimulationBarrier& Simulation,
	TFunction<EAGX_KeepContactPolicy(double TimeStamp, FShapeContactBarrier&)> ImpactCallback,
	TFunction<EAGX_KeepContactPolicy(double TimeStamp, FShapeContactBarrier&)> ContactCallback,
	TFunction<void(double TimeStamp, FAnyShapeBarrier&, FAnyShapeBarrier&)> SeparationCallback)
{
	// Create the AGX Dynamics step event listener and forward the callbacks to the constructor.
	//
	// There is currently no non-Barrier module representation of the Step Event Listener, so we
	// just create it and let the Simulation be the sole owner of its lifetime. A proper Step Event
	// Listener implementation in AGX Dynamics for Unreal would do something more here.
	new ContactEventListener(Simulation, ImpactCallback, ContactCallback, SeparationCallback);
}
