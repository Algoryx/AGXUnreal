// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AMOR/AGX_ShapeContactMergeSplitProperties.h"
#include "AGX_NativeOwner.h"
#include "AGX_SimpleMeshComponent.h"
#include "Contacts/AGX_ShapeContact.h"
#include "Shapes/AGX_ShapeEnums.h"
#include "Shapes/ShapeBarrier.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"

#include "AGX_ShapeComponent.generated.h"

class UAGX_ShapeMaterial;
class UBodySetup;
class UMaterial;

UCLASS(
	ClassGroup = "AGX", Category = "AGX", Abstract, Meta = (BlueprintSpawnableComponent),
	Hidecategories = (Cooking, Collision, Input, LOD, Physics, Replication))
class AGXUNREAL_API UAGX_ShapeComponent : public UAGX_SimpleMeshComponent, public IAGX_NativeOwner
{
	GENERATED_BODY()

public:
	UAGX_ShapeComponent();

	/**
	 * Defines physical properties of both the surface and the bulk of this shape.
	 *
	 * Surface properties do for example greatly affect frictional forces.
	 *
	 * Bulk properties have impact on collision forces but also on Rigid Body mass.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Shape")
	UAGX_ShapeMaterial* ShapeMaterial;

	UFUNCTION(BlueprintCallable, Category = "AGX Shape")
	bool SetShapeMaterial(UAGX_ShapeMaterial* InShapeMaterial);

	/**
	 * Toggle to enable or disable collision generation against this shape.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shape")
	bool bCanCollide = true;

	UFUNCTION(BlueprintCallable, Category = "AGX Shape")
	void SetCanCollide(bool CanCollide);

	UFUNCTION(BlueprintCallable, Category = "AGX Shape")
	bool GetCanCollide() const;

	/**
	 * List of collision groups that this shape component is part of.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shape")
	TArray<FName> CollisionGroups;

	/**
	 * Determines whether this shape should act as a sensor.
	 *
	 * A sensor participates in collision detection, but the contact data generated is not passed
	 * to the solver and has no influence on the simulation. Set SensorType to control how much
	 * contact data is generated for a sensor.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shape Contacts")
	bool bIsSensor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX AMOR")
	FAGX_ShapeContactMergeSplitProperties MergeSplitProperties;

	UFUNCTION(BlueprintCallable, Category = "AGX AMOR")
	void CreateMergeSplitProperties();

	/**
	 * Determines the sensor type. Only relevant if the Is Sensor property is checked.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shape Contacts", Meta = (EditCondition = "bIsSensor"))
	TEnumAsByte<enum EAGX_ShapeSensorType> SensorType = EAGX_ShapeSensorType::ContactsSensor;

	/**
	 * Enable or disable this shape as a sensor.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contacts")
	void SetIsSensor(bool IsSensor);

	/**
	 * Returns true if this shape is a sensor, otherwise false.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contacts")
	bool GetIsSensor() const;

	/**
	 * Get all shape contacts for this shape.
	 *
	 * Important: The data returned is only valid during a single simulation step. This function
	 * must therefore be called each tick that the contact data is accessed.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contacts")
	TArray<FAGX_ShapeContact> GetShapeContacts() const;

	/**
	 * Re-creates (or destroys) the triangle mesh data for the visual representation of the shape to
	 * match the physical definition of the shape.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Shape")
	void UpdateVisualMesh();

	/**
	 * Get the Native Barrier for this shape. Will return nullptr if this Shape doesn't have an
	 * associated AGX Dynamics object yet.
	 *
	 * @return The Native Barrier for this shape, or nullptr if there is no native object.
	 */
	virtual FShapeBarrier* GetNative()
		PURE_VIRTUAL(UAGX_ShapeComponent::GetNative, return nullptr;);

	/**
	 * Get the Native Barrier for this shape. Will return nullptr if this Shape doesn't have an
	 * associated AGX Dynamics object yet.
	 *
	 * @return The Native Barrier for this shape, or nullptr if there is no native object.
	 */
	virtual const FShapeBarrier* GetNative() const
		PURE_VIRTUAL(UAGX_ShapeComponent::GetNative, return nullptr;);

	/** Subclasses that overrides this MUST invoke the parent's version! */
	virtual void UpdateNativeProperties();

	/**
	 * Get the Native Barrier for this shape. Create the native AGX Dynamics object if it does not
	 * already exist.
	 *
	 * @return The Native Barrier for this shape.
	 */
	virtual FShapeBarrier* GetOrCreateNative()
		PURE_VIRTUAL(UAGX_ShapeComponent::GetOrCreateNative, return nullptr;);

	// ~Begin IAGX_NativeObject interface.
	virtual bool HasNative() const override;
	virtual uint64 GetNativeAddress() const override;
	virtual void SetNativeAddress(uint64 NativeAddress) override;
	// ~End IAGX_NativeObject interface.

	//~ Begin UActorComponent Interface
	virtual TStructOnScope<FActorComponentInstanceData> GetComponentInstanceData() const override;
	virtual void OnRegister() override;
	//~ End UActorComponent Interface

	//~ Begin USceneComponent Interface
	virtual void OnUpdateTransform(
		EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport) override;
	virtual void OnAttachmentChanged() override;
	//~ End USceneComponent Interface

	UFUNCTION(BlueprintCallable, Category = "AGX Shape")
	void AddCollisionGroup(const FName& GroupName);

	UFUNCTION(BlueprintCallable, Category = "AGX Shape")
	void RemoveCollisionGroupIfExists(const FName& GroupName);

	// ~Begin UObject interface.
	virtual void PostInitProperties() override;
	virtual void PostLoad() override; // When loaded in Editor or Game
#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& Event) override;
	void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
#endif
	// ~End UObject interface.

	// ~Begin UActorComponent interface.
	virtual void OnComponentCreated() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	// ~End UActorComponent interface.

	/**
	 * Additional Unreal Collision Geometry to use for this Shape.
	 * Does not affect AGX Dynamics, but can be used to get support for e.g. LineTrace (Query) or
	 * use as a blocking volume against Chaos physics objects (Physics). Supported for all AGX
	 * primitive Shapes. Not supported for Trimesh Shapes.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Shape", AdvancedDisplay)
	TEnumAsByte<enum ECollisionEnabled::Type> AdditionalUnrealCollision {
		ECollisionEnabled::QueryOnly};

	/*
	 * The import Guid of this Component. Only used by the AGX Dynamics for Unreal import system.
	 * Should never be assigned manually.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "AGX Dynamics Import Guid")
	FGuid ImportGuid;

protected:
	/**
	 * Get a pointer to the actual member Barrier object. This will never return nullptr. The
	 * returned Barrier may be empty.
	 *
	 * @return Pointer to the member Barrier object.
	 */
	virtual FShapeBarrier* GetNativeBarrier()
		PURE_VIRTUAL(UAGX_ShapeComponent::GetNativebarrier, return nullptr;);

	virtual const FShapeBarrier* GetNativeBarrier() const
		PURE_VIRTUAL(UAGX_ShapeComponent::GetNativebarrier, return nullptr;);

	/**
	 * Clear the reference pointer held by this Shape Component. May only be called when there is a
	 * Native to release.
	 */
	virtual void ReleaseNative() PURE_VIRTUAL(UAGX_ShapeComponent::ReleaseNative, );

#if WITH_EDITOR
	/**
	 * Should be overridden by subclasses and return whether changing the
	 * value of the specified property will need an update of the visual mesh.
	 *
	 * For struct properties, MemberPropertyName is the struct member name and,
	 * PropertyName is the name of the specific member of the struct. For other
	 * properties, both names are the same.
	 *
	 * Subclass must invoke the Super class's implementation and use its result
	 * with a logical OR!
	 */
	virtual bool DoesPropertyAffectVisualMesh(
		const FName& PropertyName, const FName& MemberPropertyName) const;
#endif

	/**
	 * Defines triangles for a visual mesh to render in Unreal Engine. Whether
	 * the mesh is always rendered or just for debug is for the user to decide.
	 * The mesh should be in local coordinates relative to this component,
	 * such that any inherited component transform (be aware of scale) that is
	 * applied after results in a rendered mesh that is correctly placed.
	 */
	virtual void CreateVisualMesh(FAGX_SimpleMeshData& OutMeshData)
	{
	} // PURE_VIRTUAL(UAGX_ShapeComponent::CreateVisualMesh, );

	/**
	 * Copy properties from the given AGX Dynamics shape into this component.
	 * Does not copy referenced attributes such as material properties.
	 * Called from each subclass' type-specific CopyFrom.
	 * @param Barrier The AGX Dynamics shape to copy from.
	 */
	void CopyFrom(const FShapeBarrier& Barrier, bool ForceOverwriteInstances = false);

	/**
	 * Updates the local transform of the native geometry to match this component's
	 * transform relative to the actor. Must be called from each subclass immediately
	 * after initializing the native geometry.
	 */
	template <typename TNative>
	void UpdateNativeLocalTransform(TNative& Native);
	// TODO: Would be easier if Native was owned by ShapeComponent, with polymorphic pointer (e.g.
	// TUniquePtr).

	/**
	 * Write the global/world Unreal transform to the local/relative transform of the native shape.
	 * This should be used only for body-less stand-alone shapes for which the native parent frame
	 * is the world coordinate system.
	 */
	virtual void UpdateNativeGlobalTransform();

	static void ApplySensorMaterial(UMeshComponent& Mesh);
	static void RemoveSensorMaterial(UMeshComponent& Mesh);

	/** Description of Unreal collision, used by e.g. Line Trace. */
	UPROPERTY(Transient, Duplicatetransient)
	TObjectPtr<UBodySetup> ShapeBodySetup;

	// ~Begin UPrimitiveComponent interface.
	virtual UBodySetup* GetBodySetup() override;
	// ~End UPrimitiveComponent interface.

	void CreateShapeBodySetupIfNeeded();

	/** These must be overriden to support Line Trace collisions. */
	virtual bool SupportsShapeBodySetup();
	virtual void UpdateBodySetup();
	virtual void AddShapeBodySetupGeometry();

private:
	bool UpdateNativeMaterial();

	// UAGX_ShapeComponent does not own the Barrier object because it cannot
	// name its type. It is instead owned by the typed subclass, such as
	// UAGX_BoxShapeComponent. Access to it is provided using virtual Get
	// functions.
};

template <typename TNative>
void UAGX_ShapeComponent::UpdateNativeLocalTransform(TNative& Native)
{
	// This assumes that the Shape's parent is the UAGX_RigidBody. If we want
	// to support other/deeper hierarchies then we need to find the total
	// transformation from the UAGX_RigidBody to this shape.
	// GetRelativeTransform may be a useful function.
	//
	// See related comment in RigidBodyComponents.cpp:GetShapes.
	Native.SetLocalPosition(GetRelativeTransform().GetLocation());
	Native.SetLocalRotation(GetRelativeTransform().GetRotation());
}
