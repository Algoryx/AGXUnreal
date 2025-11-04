#pragma once

// AGX Dynamics for Unreal includes.
#include "Contacts/AGX_ContactEnums.h"

// AGX Dynamics includes.
// Note the BeginAGXIncludes.h and EndAGXIncludes.h wrapping the AGX Dynamics header files.
#include "BeginAGXIncludes.h"
#include <agxSDK/ContactEventListener.h>
#include "EndAGXIncludes.h"

// Unreal Engine includes.
#include "Templates/Function.h"

struct FShapeBarrier;
class FShapeContactBarrier;
class FSimulationBarrier;
struct FAnyShapeBarrier;

/**
 * The AGX Dynamics Contact Event Listener. Since we are in the Private folder of the Barrier module
 * it is OK to include and use AGX Dynamics header files and types.
 */
class ContactEventListener : public agxSDK::ContactEventListener
{
public:
	/**
	 * Create a new Contact Event Listener in the given Simulation, and call the given callbacks
	 * when the Simulation reports an impact, contact, or separation.
	 *
	 * @param Simulation The Simulation to which the Contact Event Listener should be added.
	 * @param ImpactCallback Callback to call when AGX Dynamcis reports an impact.
	 * @param ContactCallback Callback to call when AGX Dynamics reports a contact.
	 * @param SeparationCallback Callback to call when AGX Dynamics reports a separation.
	 */
	ContactEventListener(
		FSimulationBarrier& Simulation,
		TFunction<EAGX_KeepContactPolicy(double, FShapeContactBarrier&)> ImpactCallback,
		TFunction<EAGX_KeepContactPolicy(double, FShapeContactBarrier&)> ContactCallback,
		TFunction<void(double, FAnyShapeBarrier&, FAnyShapeBarrier&)> SeparationCallback);

	//~ Begin agxSDK::ContactEventListener interface.
	virtual KeepContactPolicy impact(
		const agx::TimeStamp& time, agxCollide::GeometryContact* geometryContact) override;
	virtual KeepContactPolicy contact(
		const agx::TimeStamp& time, agxCollide::GeometryContact* geometryContact) override;
	virtual void separation(
		const agx::TimeStamp& time, agxCollide::GeometryPair& geometryPair) override;
	//~ End agxSDK::ContactEventListener interface.

private: // Callback to call when AGX Dynamics reports an impact, contact, or separation.
	TFunction<EAGX_KeepContactPolicy(double, FShapeContactBarrier&)> ImpactCallback;
	TFunction<EAGX_KeepContactPolicy(double, FShapeContactBarrier&)> ContactCallback;
	TFunction<void(double, FAnyShapeBarrier&, FAnyShapeBarrier&)> SeparationCallback;
};
