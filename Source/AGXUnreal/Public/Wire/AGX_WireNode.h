// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Wire/AGX_WireEnums.h"
#include "Wire/WireNodeBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_WireNode.generated.h"

/**
 * A FAGX_WireNode is a handle to an AGX Dynamics Wire Node. It does not own the underlying node,
 * the Wire does that, and since wire nodes are created and destroyed throughout the simulation it
 * is safest to re-fetch the nodes each frame through the Wire Component. Some nodes, such as Eye
 * and Body Fixed, are persistent and guaranteed to not be removed by the wire resolution updates.
 */
USTRUCT(BlueprintType, Category = "AGX Dynamics")
struct AGXUNREAL_API FAGX_WireNode
{
	GENERATED_BODY()

public:
	FAGX_WireNode() = default;
	FAGX_WireNode(const FAGX_WireNode& InOther);
	FAGX_WireNode(FWireNodeBarrier&& InBarrier);

	bool HasNative() const;

	FWireNodeBarrier* GetNative();
	const FWireNodeBarrier* GetNative() const;

	FAGX_WireNode& operator=(const FAGX_WireNode& InOther);

	/// @return The world location of this node.
	FVector GetWorldLocation() const;

	/// @return The location of this node relative to the body, or the world if there is no body.
	FVector GetLocalLocation() const;

	/// @return The type of this node.
	EWireNodeType GetType() const;

	/**
	 * Get the Rigid Body this node is attached to. Sometimes this is a body implicitly created by
	 * the wire and sometimes it is a body explicitly set on e.g. a Body Fixed or an Eye node.
	 *
	 * @return The Rigid Body that this node is attached to.
	 */
	FRigidBodyBarrier GetRigidBody() const;

private:
	FWireNodeBarrier Barrier;
};

UCLASS()
class AGXUNREAL_API UAGX_WireNode_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Wire Rendering")
	static FVector GetWorldLocation(UPARAM(Ref) FAGX_WireNode& Node);

	UFUNCTION(BlueprintPure, Category = "Wire Rendering")
	static EWireNodeType GetType(UPARAM(Ref) FAGX_WireNode& Node);
};
