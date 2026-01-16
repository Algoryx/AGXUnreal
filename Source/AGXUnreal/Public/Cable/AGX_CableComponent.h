// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_NativeOwner.h"
#include "Cable/AGX_CableNodeInfo.h"
#include "Cable/AGX_CableRouteNode.h"
#include "Cable/CableBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"

#include "AGX_CableComponent.generated.h"

class UAGX_CableProperties;
class UInstancedStaticMeshComponent;
class UMaterialInterface;

struct FAGX_ImportContext;

/**
 * A frame attached to a RigidBody with an optional relative transform.
 * During runtime, it is possible to get its position, velocity, angular velocity, acceleration
 * etc.
 */
UCLASS(ClassGroup = "AGX", Category = "AGX", Meta = (BlueprintSpawnableComponent))
class AGXUNREAL_API UAGX_CableComponent : public USceneComponent, public IAGX_NativeOwner
{
	GENERATED_BODY()

public:
	UAGX_CableComponent();

	/**
	 * The radius of the Cable [cm].
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Cable",
		Meta = (ClampMin = "0", UIMin = "0", ExposeOnSpawn))
	double Radius {3.0};

	/**
	 * The length of each Cable segments, or lumped elements [cm].
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadOnly, Category = "AGX Cable",
		Meta = (ClampMin = "0.1", UIMin = "0.1", ExposeOnSpawn))
	double SegmentLength {30.0};

	/**
	 * Scale to apply to the radius when rendering the Cable.
	 * By increasing the Rander Radius Scale it is possible to make the Cable larger on-screen
	 * without affecting the simulation behavior.
	 *
	 * This setting affects rendering only, it does not change the simulation behavior or collision
	 * shape of the Cable.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Cable")
	double RenderRadiusScale {1.0};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Cable")
	UAGX_CableProperties* CableProperties;

	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	bool SetCableProperties(UAGX_CableProperties* Properties);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Cable")
	UMaterialInterface* RenderMaterial {nullptr};

	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	void SetRenderMaterial(UMaterialInterface* Material);

	/**
	 * An array of route nodes that are used to initialize the Cable.
	 * At BeginPlay these nodes are used to create simulation nodes, after that these nodes are not
	 * used and will not be updated during Play.
	 */
	UPROPERTY(
		EditAnywhere, BlueprintReadWrite, Category = "AGX Cable Route", Meta = (ExposeOnSpawn))
	TArray<FAGX_CableRouteNode> RouteNodes;

	/**
	 * Create a new default-constructed routing node at the end of the Cable.
	 */
	FAGX_CableRouteNode& AddNode();

	/**
	 * Create a new default-constructed routing node at the end of the Cable.
	 */
	FAGX_CableRouteNode& AddNode(int32& OutIndex);

	/**
	 * Create a new default-constructed routing node at the end of the Cable.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Cable Route", Meta = (DisplayName = "Add Node"))
	UPARAM(Ref) FAGX_CableRouteNode& CreateNode(int32& OutIndex);
	// Extra Blueprint Callable function with different name because Blueprint does not support
	// function overloading.

	/**
	 * Add a new route node to the Cable.
	 *
	 * This should be called before BeginPlay since route nodes are only used during initialization.
	 *
	 * @param InNode The node to add.
	 */
	FAGX_CableRouteNode& AddNode(const FAGX_CableRouteNode& InNode);

	/**
	 * Add a new route node to the Cable.
	 *
	 * This should be called before BeginPlay since route nodes are only used during initialization.
	 *
	 * @param InNode The node to add.
	 * @param OutIndex The index in the Route Nodes array at which the new node was placed.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Cable Route")
	UPARAM(Ref) FAGX_CableRouteNode& AddNode(const FAGX_CableRouteNode& InNode, int32& OutIndex);

	/**
	 * Add a new route node at the end of the Cable at the given location relative to the Cable
	 * Component.
	 *
	 * @param InLocation Location of the new route node relative to the Cable Component.
	 */
	FAGX_CableRouteNode& AddNodeAtLocation(FVector InLocation);

	/**
	 * Add a new route node at the end of the Cable at the given location relative to the Cable
	 * Component.
	 *
	 * @param InLocation The location of the node, relative to the Cable Component.
	 * @param OutIndex Index in Route Nodes where the new node is stored.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Cable Route")
	UPARAM(Ref) FAGX_CableRouteNode& AddNodeAtLocation(FVector InLocation, int32& OutIndex);

	/**
	 * Add a default-constructed route node at the designated index in the route array, pushing all
	 * subsequent nodes one index.
	 *
	 * The index must be either within the current Route Nodes array, or one-past end.
	 *
	 * @param InNode The route node to add.
	 * @param InIndex The place in the route node array to add the node at.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Cable Route")
	UPARAM(Ref)
	FAGX_CableRouteNode& AddNodeAtIndex(const FAGX_CableRouteNode& InNode, int32 InIndex);

	/**
	 * Add a default-constructed route node, placed at the given local location, at the designated
	 * index in the route array, pushing all subsequent nodes one index.
	 *
	 * The index must be either within the current Route Nodes array, or one-past end.
	 *
	 * @param InLocation The location of the new node relative to the Cable Component.
	 * @param InIndex The place in the route node array to add the new node at.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Cable Route")
	UPARAM(Ref)
	FAGX_CableRouteNode& AddNodeAtLocationAtIndex(FVector InLocation, int32 InIndex);

	/**
	 * Overwrite the node at the given index with the new node.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Cable Route")
	void SetNode(int32 InIndex, UPARAM(Ref) const FAGX_CableRouteNode InNode);

	/**
	 * Remove the route node at the given index.
	 * @param InIndex The index of the node to remove.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Cable Route")
	void RemoveNode(int32 InIndex);

	/**
	 * Set the local location of a routing node.
	 *
	 * The local location is relative to the parent set on the node's Frame. If no parent has been
	 * set then the parent is assumed to be the Cable Component.
	 *
	 * @param InIndex The index of the node to modify.
	 * @param InLocation The new local location of the node
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Cable Route")
	void SetNodeLocalLocation(int32 InIndex, FVector InLocation);

	/**
	 * Set the location of the node at the given index. The location is relative to the Cable
	 * Component. If the routing node has a parent then a local location relative to that parent is
	 * computed that places the routing node at the requested location relative to the Cable
	 * Component.
	 *
	 * Use SetNodeLocalLocation to position the node relative to its Frame parent.
	 *
	 * @param InIndex The index of the node to remove.
	 * @param InLocation The new local location for the node.
	 * @see SetNodeLocalLocation
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Cable Route")
	void SetNodeLocation(int32 InIndex, FVector InLocation);

	/*
	 * The import Guid of this Component. Only used by the AGX Dynamics for Unreal import system.
	 * Should never be assigned manually.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Dynamics Import Guid")
	FGuid ImportGuid;

	/*
	 * The import name of this Component. Only used by the AGX Dynamics for Unreal import system.
	 * Should never be assigned manually.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AGX Dynamics Import Name")
	FString ImportName;

	/**
	 * Returns an array of Node Info structs containing information about each Node in the Cable.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	TArray<FAGX_CableNodeInfo> GetNodeInfo() const;

	/**
	 * Mark visuals for this Cable Component dirty. The Visuals will be updated to match to the
	 * current Cable state.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Cable")
	void MarkVisualsDirty();

	void CopyFrom(const FCableBarrier& Barrier, FAGX_ImportContext* Context);

	FCableBarrier* GetNative();
	const FCableBarrier* GetNative() const;

	FCableBarrier* GetOrCreateNative();

	//~ Begin IAGX_NativeObject interface.
	virtual bool HasNative() const override;
	virtual uint64 GetNativeAddress() const override;
	virtual void SetNativeAddress(uint64 NativeAddress) override;
	//~ End IAGX_NativeObject interface.

	//~ Begin UActorComponent interface
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual TStructOnScope<FActorComponentInstanceData> GetComponentInstanceData() const override;
	virtual void TickComponent(
		float DeltaTime, ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction) override;
	//~ End UActorComponent interface

#if WITH_EDITOR
	// ~Begin UObject interface.
	virtual void PostInitProperties() override;
	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
	virtual void OnRegister() override;
	// ~End UObject interface.
#endif

private:
#if WITH_EDITOR
	void InitPropertyDispatcher();
	bool DoesPropertyAffectVisuals(const FName& MemberPropertyName) const;
#endif
	void UpdateNativeProperties();
	bool UpdateNativeCableProperties();
	void CreateNative();
	void CreateVisuals();
	bool ShouldRenderSelf() const;
	void UpdateVisuals();
	void RenderSelf();

#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* InProperty) const override;
#endif
	TArray<FTransform> VisualCylinderTransformsPrev;
	TArray<FTransform> VisualSphereTransformsPrev;

	TObjectPtr<UInstancedStaticMeshComponent> VisualCylinders;
	TObjectPtr<UInstancedStaticMeshComponent> VisualSpheres;
	FCableBarrier NativeBarrier;
};
