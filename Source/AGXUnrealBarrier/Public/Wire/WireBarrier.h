// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Wire/WireRenderIteratorBarrier.h"

// Standard library includes.
#include <memory>

class FShapeMaterialBarrier;
struct FWireRef;
class FWireNodeBarrier;
class FWireWinchBarrier;
class FWireParameterControllerBarrier;

class AGXUNREALBARRIER_API FWireBarrier
{
public:
	FWireBarrier();
	FWireBarrier(std::unique_ptr<FWireRef> Native);
	FWireBarrier(FWireBarrier&& Other);
	~FWireBarrier();

	/** Set the radius of the wire [cm]. */
	void SetRadius(float Radius);

	/** Get the radius of the wire [cm]. */
	float GetRadius() const;

	/** Get the maximum resolution of the wire [nodes/cm]. */
	void SetResolutionPerUnitLength(float InResolution);

	/** Set the maximum resolution of the wire [nodes/cm]. */
	float GetResolutionPerUnitLength() const;

	void SetEnableCollisions(bool CanCollide);
	bool GetEnableCollisions() const;

	void AddCollisionGroup(const FName& GroupName);
	void AddCollisionGroups(const TArray<FName>& GroupNames);
	TArray<FName> GetCollisionGroups() const;
	void RemoveCollisionGroup(const FName& GroupName);

	void SetLinearVelocityDamping(double Damping);
	double GetLinearVelocityDamping() const;

	void SetMaterial(const FShapeMaterialBarrier& Material);
	void ClearMaterial();

	FShapeMaterialBarrier GetMaterial() const;

	bool GetRenderListEmpty() const;

	void AddRouteNode(FWireNodeBarrier& RoutingNode);
	void AddWinch(FWireWinchBarrier& Winch);

	/**
	 * Get the very first node in the wire. This may be before the node pointed to by Get Render
	 * Begin Iterator, for example when the begin side of the wire is attached to a winch.
	 */
	FWireNodeBarrier GetFirstNode() const;

	/**
	 * Get the very last node in the wire. This may be after last node reachable from the Get Render
	 * Begin Iterator, for example when the end side of the wire is attached to a winch.
	 */
	FWireNodeBarrier GetLastNode() const;

	FWireWinchBarrier GetBeginWinch() const;
	FWireWinchBarrier GetEndWinch() const;
	FWireWinchBarrier GetWinch(EWireSide Side) const;

	bool IsInitialized() const;

	/**
	 * @return The length of the wire not including wire inside any winches [cm].
	 */
	double GetRestLength() const;

	/** @return The mass of the wire [kg]. */
	double GetMass() const;

	/** Get the tension at the begin side of the wire [N] */
	double GetTension() const;

	FWireParameterControllerBarrier GetParameterController() const;

	/**
	 * Attach a winch to a free end of this wire.
	 *
	 * If an object is attached to begin, it will be detached, and this winch controller will be
	 * attached at this position instead.
	 *
	 * Parameters
	 * @param Winch	Winch to attach.
	 * @param bBegin True if the winch should be attached at begin, false at end.
	 * @return True if the Winch was attached, false otherwise.
	 */
	bool Attach(FWireWinchBarrier& Winch, bool bBegin);

	/**
	 * Detach begin or end of this wire (if attached to something).
	 *
	 * @param bBegin If true begin of this wire will be detached, otherwise end.
	 * @return True if a detach was performed, false otherwise.
	 */
	bool Detach(bool bBegin);

	bool Detach(FWireWinchBarrier& Winch);

	FWireRenderIteratorBarrier GetRenderBeginIterator() const;
	FWireRenderIteratorBarrier GetRenderEndIterator() const;

	bool IsLumpedNode(const FWireNodeBarrier& Node) const;
	bool IsLumpedNode(const FWireRenderIteratorBarrier& Node) const;

	FString GetName() const;

	FGuid GetGuid() const;

	void AllocateNative(float Radius, float ResolutionPerUnitLength);
	bool HasNative() const;
	FWireRef* GetNative();
	const FWireRef* GetNative() const;

	uintptr_t GetNativeAddress() const;

	void SetNativeAddress(uintptr_t NativeAddress);

	void ReleaseNative();


private:
	FWireBarrier(const FWireBarrier&) = delete;
	void operator=(const FWireBarrier&) = delete;

private:
	std::unique_ptr<FWireRef> NativeRef;
};
