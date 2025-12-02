// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_RealInterval.h"

// Standard library includes.
#include <memory>

struct FRigidBodyBarrier;
class FWireBarrier;
struct FWireWinchRef;

class AGXUNREALBARRIER_API FWireWinchBarrier
{
public:
	FWireWinchBarrier();
	FWireWinchBarrier(std::unique_ptr<FWireWinchRef> Native);
	FWireWinchBarrier(FWireWinchBarrier&& Other);
	virtual ~FWireWinchBarrier();

	/// The body that the winch is attached to. Will be empty when attached to the world.
	FRigidBodyBarrier GetRigidBody() const;

/// @todo Don't know how to read this from the AGX Dynamics API.
#if 0
	/// The position of the winch on the body it's attached to, or in world space if there is no
	/// body.
	FVector GetLocalPosition() const;
#endif

	/**
	 * @return The direction of the winch on the body it's attached to, or in world space if there
	 * is no body.
	 */
	FVector GetNormal() const;

	/**
	 * @return The location of the winch in the body's frame, or world space if there is no body.
	 */
	FVector GetLocation() const;

	/*
	 * Set the length of wire that is held inside the winch. This will create new wire, not move
	 * free wire into the winch.
	 */
	void SetPulledInWireLength(double InPulledInLength);

	/**
	 * The length of wire that the winch contains currently.
	 * This will decrease during routing/initialization if Auto Feed is enabled.
	 */
	double GetPulledInWireLength() const;

	/**
	 * Decide if wire should be taken from the winch during routing, or if the routed wire is in
	 * addition to the the initial pulled in length. Only used during initialization.
	 * @param bAutoFeed True to take from the winch, false to add wire in addition to the winch.
	 */
	void SetAutoFeed(bool bAutoFeed);

	bool GetAutoFeed() const;

	/// Maximum force to push or pull the wire.
	FAGX_RealInterval GetForceRange() const;

	/**
	 * Set the maximum forces that the winch may use to haul in or pay out wire.
	 * The lower end of the range must be negative or zero and is the maximum force to haul in.
	 * The upper end of the range must be positive or zero and is the maximum force to pay out.
	 */
	void SetForceRange(const FAGX_RealInterval& InForceRange);

	void SetForceRange(double MinForce, double MaxForce);

	/// The ability of the winch to slow down the wire when the brake is enabled.
	FAGX_RealInterval GetBrakeForceRange() const;

	void SetBrakeForceRange(const FAGX_RealInterval& InBrakeForceRange);

	void SetBrakeForceRange(double MinForce, double MaxForce);

	/**
	 * The speed that the winch tries to haul in or pay out wire with.
	 * Positive values is paying out.
	 * Negative values is hauling in.
	 */
	double GetSpeed() const;

	/**
	 * Set the speed that the winch tries to haul in or pay out wire with.
	 * Positive values is paying out.
	 * Negative values is hauling in.
	 */
	void SetSpeed(double InTargetSpeed);

	/**
	 * The current speed of the winch motor.
	 * Positive values is paying out.
	 * Negative values is hauling in.
	 */
	double GetCurrentSpeed() const;

	/// \return The force that the motor is currently applying.
	double GetCurrentForce() const;

	/// \return The force that the brake is currently applying.
	double GetCurrentBrakeForce() const;

	bool HasWire() const;

// Not yet implemented.
#if 0
	FWireBarrier GetWire() const;
#endif

	FGuid GetGuid() const;

	void AllocateNative(
		const FRigidBodyBarrier* Body, const FVector& LocalLocation, const FVector& LocalNormal,
		double PulledInLength);

	void AllocateNative(float Radius, float ResolutionPerUnitLength);
	bool HasNative() const;
	FWireWinchRef* GetNative();
	const FWireWinchRef* GetNative() const;

	uintptr_t GetNativeAddress() const;

	void SetNativeAddress(uintptr_t NativeAddress);

	void IncrementRefCount() const;
	void DecrementRefCount() const;

	void ReleaseNative();


private:
	FWireWinchBarrier(const FWireBarrier&) = delete;
	void operator=(const FWireWinchBarrier&) = delete;

private:
	std::unique_ptr<FWireWinchRef> NativeRef;
};
