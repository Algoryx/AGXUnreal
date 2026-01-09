// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Frame.h"
#include "AGX_RigidBodyReference.h"
#include "Cable/AGX_CableEnums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_CableNode.generated.h"

/**
 * Cable nodes are used to specify the route of a cable. Each node has a location and
 * BodyFixed nodes may use orientation. Free nodes does not use rotation. Some members are only
 * used for some node types, such as RigidBody which is only used by BodyFixed nodes.
 */
USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_CableNode
{
	GENERATED_BODY();

	/**
	 * The type of node, e.g., Free, Body Fixed.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Cable")
	EAGX_CableNodeType NodeType {EAGX_CableNodeType::Free};

	/**
	 * The location of the Cable node. Relative to the Cable by default but any parent Scene Component
	 * may be set.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Cable")
	FAGX_Frame Frame;

	/**
	 * The Rigid Body that a Body Fixed node should be attached to.
	 * Ignored for other node types.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Cable")
	FAGX_RigidBodyReference RigidBody;

	FAGX_CableNode()
	{
	}

	FAGX_CableNode(EAGX_CableNodeType InNodeType)
		: NodeType(InNodeType)
	{
	}

	FAGX_CableNode(const FVector& InLocation)
	{
		Frame.LocalLocation = InLocation;
	}

	void SetBody(UAGX_RigidBodyComponent* Body);
};
