// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_NativeOwner.h"
#include "Wire/WireLinkBarrier.h"

// Unreal Engine includes.
#include "AGX_Real.h"
#include "Components/SceneComponent.h"

#include "AGX_WireLinkComponent.generated.h"

class UAGX_RigidBodyComponent;
class UAGX_WireComponent;

/**
 * Wraps an agxWire::Link and exposes it as an Unreal Engine Scene Component.
 *
 * An agxWire::Link connects the ends of two separate wires through a common rigid body,
 * routing physical tension from one wire to the other.
 *
 * Usage:
 *  1. Attach this component as a child of the UAGX_RigidBodyComponent that the link wraps.
 *  2. On each UAGX_WireComponent that should pass through the link, add a route node with
 *     NodeType = Connecting and point its RigidBody reference at the same body.
 *
 * The component can also be used standalone (no wires) to add stabilization constraints
 * to a body that sits between two heavy bodies connected via regular AGX constraints.
 *
 * ⚠️ NOTE: agxWire::Link requires the AgX-WireLink license module. This component will
 * log an error at BeginPlay and do nothing if the module is not in the active license.
 * See the implementation plan for details: Documentation/AGXWireSimulatingWires/
 * agxwire_connecting_node_implementation_plan.md
 */
UCLASS(
	ClassGroup = "AGX", BlueprintType, Meta = (BlueprintSpawnableComponent),
	HideCategories = ("Rendering", "Physics", "LOD", "Activation", "Cooking", "Collision",
					  "Navigation", "ComponentTick", "ComponentReplication", "Events",
					  "AssetUserData", "Mobile"))
class AGXUNREAL_API UAGX_WireLinkComponent : public USceneComponent, public IAGX_NativeOwner
{
	GENERATED_BODY()

public:
	UAGX_WireLinkComponent();

	/**
	 * Runtime list of Wire Components that have routed through this Link.
	 *
	 * Populated automatically during BeginPlay as wire components call RegisterConnectedWire().
	 * Read-only from Blueprints.
	 */
	UPROPERTY(BlueprintReadOnly, Transient, Category = "AGX Wire Link")
	TArray<UAGX_WireComponent*> ConnectedWires;

	/**
	 * Returns the UAGX_RigidBodyComponent this link wraps, found by walking the attachment
	 * hierarchy upward. Returns nullptr if this component is not attached to a
	 * UAGX_RigidBodyComponent (directly or transitively).
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Wire Link")
	UAGX_RigidBodyComponent* GetRigidBody() const;

	/**
	 * Radius [cm] applied to every ConnectingNode created for wires that route through this link.
	 *
	 * Each WireLinkComponent carries a single radius value that is forwarded to all connecting
	 * nodes when the simulation is initialized (BeginPlay). A value of 0 uses the default AGX
	 * connecting-node radius (no override).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Wire Link",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real Radius = 0.0;

	/**
	 * Called by UAGX_WireComponent during its own CreateNative to register itself as a wire
	 * that routes through this link. Entries are deduplicated.
	 */
	void RegisterConnectedWire(UAGX_WireComponent* Wire);

	//~ Begin IAGX_NativeOwner interface.
	virtual bool HasNative() const override;
	virtual uint64 GetNativeAddress() const override;
	virtual void SetNativeAddress(uint64 NativeAddress) override;
	//~ End IAGX_NativeOwner interface.

	//~ Begin UActorComponent interface.
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual TStructOnScope<FActorComponentInstanceData> GetComponentInstanceData() const override;
	//~ End UActorComponent interface.

	/**
	 * Returns the FWireLinkBarrier if one has been allocated, nullptr otherwise.
	 */
	FWireLinkBarrier* GetNative();

	/**
	 * Returns the FWireLinkBarrier if one has been allocated, nullptr otherwise.
	 */
	const FWireLinkBarrier* GetNative() const;

	/**
	 * Returns the barrier, allocating the native if it does not yet exist.
	 *
	 * Safe to call from UAGX_WireComponent::CreateNative — the link native will be created
	 * on demand and the link will add itself to the simulation when its own BeginPlay runs.
	 * Returns nullptr if the native could not be created (missing license or unresolved body).
	 */
	FWireLinkBarrier* GetOrCreateNative();

private:
	/**
	 * Allocate the agxWire::Link native. Resolves the body via attachment hierarchy and
	 * creates the barrier. Logs an error and does nothing if no body is found.
	 */
	void CreateNative();


	FWireLinkBarrier NativeBarrier;
};
