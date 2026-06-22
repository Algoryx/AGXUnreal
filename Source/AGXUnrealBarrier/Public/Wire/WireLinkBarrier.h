// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Math/Vector.h"
#include "Misc/Guid.h"

// Standard library includes.
#include <memory>

struct FRigidBodyBarrier;
struct FWireLinkRef;
class FWireBarrier;

class AGXUNREALBARRIER_API FWireLinkBarrier
{
public:
	FWireLinkBarrier();
	FWireLinkBarrier(FWireLinkBarrier&& Other);
	~FWireLinkBarrier();

	/**
	 * Allocate the native agxWire::Link wrapping the given Rigid Body.
	 * The body must already have a native allocated.
	 */
	void AllocateNative(FRigidBodyBarrier& Body);

	bool HasNative() const;
	void ReleaseNative();

	FWireLinkRef* GetNative();
	const FWireLinkRef* GetNative() const;

	uintptr_t GetNativeAddress() const;
	void SetNativeAddress(uintptr_t NativeAddress);

	/**
	 * Register which end of the given wire is connected to this link, and at which position
	 * on the link body (in link body local space [cm]).
	 *
	 * @param Wire        The wire to register.
	 * @param LocalPosition  Attachment point in link-body local space [cm].
	 * @param bWireBegin  True if this is the WIRE_BEGIN connection, false for WIRE_END.
	 */
	void Connect(FWireBarrier& Wire, const FVector& LocalPosition, bool bWireBegin);

	/**
	 * Insert this link into the wire's route by calling wire->add(link).
	 * This is what actually creates the ConnectingNode inside the wire.
	 * Call Connect() for this wire before calling this function.
	 */
	void AddToWireRoute(FWireBarrier& Wire);

	/**
	 * Set the radius [cm] on the ConnectingNode that was created for the given wire.
	 *
	 * Must be called after AddToWireRoute() so that the ConnectingNode already exists.
	 * Has no effect if the link has no connection for the given wire.
	 *
	 * Background: the AGX Link API (link->connect + wire->add(link)) has no radius
	 * parameter — AGX always constructs the ConnectingNode with radius 0 internally.
	 * This method is therefore the only way to set the radius for Link-managed
	 * ConnectingNodes; it is always a post-creation adjustment rather than a
	 * constructor argument.
	 *
	 * @param Wire       The wire whose connecting node should be updated.
	 * @param RadiusCm   The desired radius in centimetres (Unreal units). Converted to
	 *                   metres before being forwarded to AGX Dynamics.
	 */
	void SetConnectingNodeRadius(FWireBarrier& Wire, double RadiusCm);

	FRigidBodyBarrier GetRigidBody() const;
	FGuid GetGuid() const;

private:
	FWireLinkBarrier(const FWireLinkBarrier&) = delete;
	void operator=(const FWireLinkBarrier&) = delete;

private:
	std::unique_ptr<FWireLinkRef> NativeRef;
};

