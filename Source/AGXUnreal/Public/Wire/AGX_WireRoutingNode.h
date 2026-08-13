// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Frame.h"
#include "AGX_Real.h"
#include "AGX_RigidBodyReference.h"
#include "Wire/AGX_WireEnums.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_WireRoutingNode.generated.h"

/**
 * Routing nodes are used to specify the initial route of a wire. Each node has a location but
 * no orientation. Some members are only used for certain node types:
 * - RigidBody is used by Eye, BodyFixed, and Connecting nodes.
 * - Radius is only meaningful for Eye nodes.
 *
 * For Connecting nodes the wire side (WIRE_BEGIN or WIRE_END) is determined automatically from
 * the node's position in the route array: index 0 → WIRE_BEGIN, last index → WIRE_END.
 * A Connecting node must therefore be either the first or the last node in the route.
 */
USTRUCT(BlueprintType)
struct AGXUNREAL_API FWireRoutingNode
{
	GENERATED_BODY();

	/**
	 * The type of wire node, e.g., Free, Eye, Body Fixed, etc.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wire")
	EWireNodeType NodeType;

	/**
	 * The location of the wire node. Relative to the wire by default but any parent Scene Component
	 * may be set.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wire")
	FAGX_Frame Frame;

	/**
	 * The Rigid Body that an Eye, Body Fixed, or Connecting node should be attached to.
	 *
	 * For Connecting nodes this must be the UAGX_RigidBodyComponent that the target
	 * UAGX_WireLinkComponent is a child of. The WireLinkComponent is found at runtime by
	 * scanning the direct children of this body component. Frame gives the wire's routing
	 * position; Frame.GetLocationRelativeTo gives the body-local connection offset [cm].
	 *
	 * Ignored for Free nodes.
	 */
	UPROPERTY(EditAnywhere, Category = "Wire")
	FAGX_RigidBodyReference RigidBody;


	/**
	 * Radius [cm] of the eye hole through which the wire slides.
	 *
	 * Controls the pulley/ring geometry at this node and influences the friction constraint
	 * arm length. A value of 0 uses the AGX default (no eye radius).
	 *
	 * Only meaningful when NodeType is Eye. Hidden and ignored for all other node types.
	 */
	UPROPERTY(
		EditAnywhere, Category = "Wire",
		Meta = (
			EditCondition = "NodeType == EWireNodeType::Eye",
			EditConditionHides,
			ClampMin = "0.0", UIMin = "0.0",
			ToolTip = "Radius [cm] of the eye hole through which the wire slides. Only used when NodeType is Eye."))
	FAGX_Real Radius {0.0};

	FWireRoutingNode()
		: NodeType(EWireNodeType::Free)
	{
	}

	FWireRoutingNode(EWireNodeType InNodeType)
		: NodeType(InNodeType)
	{
	}

	FWireRoutingNode(const FVector& InLocation)
		: NodeType(EWireNodeType::Free)
	{
		Frame.LocalLocation = InLocation;
	}

	void SetBody(UAGX_RigidBodyComponent* Body);

	bool IsValid() const;

	bool Serialize(FArchive& Archive);

private:
	/**
	 * The location of this node relative to the Wire Component [cm].
	 */
	UPROPERTY(Meta = (DeprecatedProperty, DeprecationMessage = "Use Frame instead."))
	FVector Location_DEPRECATED {FVector::ZeroVector};
};

/// A globally accessible Routing Node that is used when a function with a node reference return
/// type need to signal a failure.
extern FWireRoutingNode InvalidRoutingNode;

template <>
struct TStructOpsTypeTraits<FWireRoutingNode> : public TStructOpsTypeTraitsBase2<FWireRoutingNode>
{
	enum
	{
		WithSerializer = true
	};
};

/**
 * This class acts as an API that exposes functions of FAGX_WireRouteNode in Blueprint Visual
 * Scripts.
 */
UCLASS()
class AGXUNREAL_API UAGX_WireRouteNode_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "AGX Wire Node")
	static UPARAM(Ref) FWireRoutingNode& SetBody(
		UPARAM(Ref) FWireRoutingNode& WireNode, UAGX_RigidBodyComponent* Body)
	{
		WireNode.SetBody(Body);
		return WireNode;
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Wire Node")
	static UPARAM(Ref) FWireRoutingNode& SetFrame(
		UPARAM(Ref) FWireRoutingNode& WireNode, UPARAM(Ref) FAGX_Frame& Frame)
	{
		WireNode.Frame = Frame;
		return WireNode;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Wire Node")
	static UPARAM(Ref) FAGX_Frame& GetFrame(UPARAM(ref) FWireRoutingNode& WireNode)
	{
		return WireNode.Frame;
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Wire Node|Frame")
	static UPARAM(Ref)
		FWireRoutingNode& SetParentName(UPARAM(Ref) FWireRoutingNode& WireNode, FName ParentName)
	{
		WireNode.Frame.Parent.Name = ParentName;
		return WireNode;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Wire Node|Frame")
	static FName GetParentName(UPARAM(Ref) FWireRoutingNode& WireNode)
	{
		return WireNode.Frame.Parent.Name;
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Wire Node|Frame")
	static UPARAM(Ref) FWireRoutingNode& SetParentOwningActor(
		UPARAM(Ref) FWireRoutingNode& WireNode, AActor* OwningActor)
	{
		WireNode.Frame.Parent.OwningActor = OwningActor;
		return WireNode;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Wire Node|Frame")
	static AActor* GetParentOwningActor(UPARAM(Ref) FWireRoutingNode& WireNode)
	{
		return WireNode.Frame.Parent.OwningActor;
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Wire Node|Frame")
	static UPARAM(Ref) FWireRoutingNode& SetLocalLocation(
		UPARAM(Ref) FWireRoutingNode& WireNode, FVector LocalLocation)
	{
		WireNode.Frame.LocalLocation = LocalLocation;
		return WireNode;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Wire Node|Frame")
	static FVector GetLocalLocation(UPARAM(Ref) FWireRoutingNode& WireNode)
	{
		return WireNode.Frame.LocalLocation;
	}

	UFUNCTION(BlueprintCallable, Category = "AGX Wire Node")
	static UPARAM(Ref) FWireRoutingNode& SetRadius(
		UPARAM(Ref) FWireRoutingNode& WireNode, double RadiusCm)
	{
		WireNode.Radius = RadiusCm;
		return WireNode;
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Wire Node")
	static double GetRadius(UPARAM(Ref) FWireRoutingNode& WireNode)
	{
		return WireNode.Radius.Value;
	}
};
