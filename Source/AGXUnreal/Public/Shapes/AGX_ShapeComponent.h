// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AMOR/AGX_ShapeContactMergeSplitProperties.h"
#include "AGX_NativeOwner.h"
#include "AGX_RigidBodyComponent.h"
#include "Shapes/AGX_SimpleMeshComponent.h"
#include "Contacts/AGX_ShapeContact.h"
#include "Shapes/AGX_ShapeEnums.h"
#include "Shapes/ShapeBarrier.h"
#include "Utilities/AGX_ObjectUtilities.h"

// Unreal Engine includes.
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

#include "AGX_ShapeComponent.generated.h"

struct FAGX_ImportContext;

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

	UFUNCTION(BlueprintCallable, Category = "AGX Shape")
	void AddCollisionGroup(FName GroupName);

	UFUNCTION(BlueprintCallable, Category = "AGX Shape")
	void RemoveCollisionGroupIfExists(FName GroupName);

	/**
	 * Determines whether this shape should act as a sensor.
	 *
	 * A sensor participates in collision detection, but the contact data generated is not passed
	 * to the solver and has no influence on the simulation. Set SensorType to control how much
	 * contact data is generated for a sensor.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shape Contacts")
	bool bIsSensor;

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
	 * Determines the sensor type. Only relevant if the Is Sensor property is checked.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Shape Contacts", Meta = (EditCondition = "bIsSensor"))
	EAGX_ShapeSensorType SensorType {EAGX_ShapeSensorType::ContactsSensor};

	/**
	 * Set the velocity of this Shape's surface in the Shape's local coordinate frame [cm/s].
	 *
	 * If this velocity is non-zero the constraints will try to achieve the relative velocity
	 * between the objects in the friction plane to be the wanted surface velocity.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AGX Shape")
	FVector SurfaceVelocity {FVector::ZeroVector};

	/**
	 * Set the velocity of this Shape's surface in the Shape's local coordinate frame [cm/s].
	 *
	 * If this velocity is non-zero the constraints will try to achieve the relative velocity
	 * between the objects in the friction plane to be the wanted surface velocity.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Shape")
	void SetSurfaceVelocity(FVector InSurfaceVelocity);

	/**
	 * Get the velocity of this Shape's surface in the Shape's local coordinate frame [cm/s].
	 *
	 * If this velocity is non-zero the constraints will try to achieve the relative velocity
	 * between the objects in the friction plane to be the wanted surface velocity.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Shape")
	FVector GetSurfaceVelocity() const;

	/**
	 * Defines the AMOR (merge split) properties for this Shape. For this to take effect, AMOR has
	 * to be enabled globally in the AGX Dynamics for Unreal project settings.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX AMOR")
	FAGX_ShapeContactMergeSplitProperties MergeSplitProperties;

	UFUNCTION(BlueprintCallable, Category = "AGX AMOR")
	void CreateMergeSplitProperties();

	/**
	 * Additional Unreal Collision Geometry to use for this Shape.
	 * Does not affect AGX Dynamics, but can be used to get support for e.g. LineTrace (Query) or
	 * use as a blocking volume against Chaos physics objects (Physics). Supported for all AGX
	 * primitive Shapes. Not supported for Trimesh Shapes.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Shape", AdvancedDisplay)
	TEnumAsByte<enum ECollisionEnabled::Type> AdditionalUnrealCollision {
		ECollisionEnabled::QueryOnly};

	/**
	 * The import Guid of this Component. Only used by the AGX Dynamics for Unreal import system.
	 * Should never be assigned manually.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "AGX Dynamics Import Guid")
	FGuid ImportGuid;

	/**
	 * Get all shape contacts for this shape.
	 *
	 * Important: The data returned is only valid during a single simulation step. This function
	 * must therefore be called each tick that the contact data is accessed.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contacts")
	TArray<FAGX_ShapeContact> GetShapeContacts() const;

	/**
	 * Recompute the local transform that the native AGX Dynamics object should have given the
	 * current Scene Component attachment hierarchy. This is done automatically on Begin Play but
	 * may be required again if the attachment hierarchy is changed, for example when a Shape
	 * Component becomes attached to a Rigid Body.
	 *
	 * @see UAGX_RigidBodyComponent::SynchronizeShapes
	 */
	void UpdateNativeLocalTransform();

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

	//~ Begin IAGX_NativeObject interface.
	virtual bool HasNative() const override;
	virtual uint64 GetNativeAddress() const override;
	virtual void SetNativeAddress(uint64 NativeAddress) override;
	//~ End IAGX_NativeObject interface.

	//~ Begin UActorComponent interface
	virtual void OnComponentCreated() override;
	virtual void OnRegister() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual TStructOnScope<FActorComponentInstanceData> GetComponentInstanceData() const override;
	//~ End UActorComponent interface

	//~ Begin USceneComponent interface
	virtual void OnUpdateTransform(
		EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport) override;
	virtual void OnAttachmentChanged() override;
	//~ End USceneComponent interface

	// ~Begin UObject interface.
	virtual void PostInitProperties() override;
	virtual void PostLoad() override; // When loaded in Editor or Game
#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& Event) override;
	void PostEditChangeChainProperty(FPropertyChangedChainEvent& Event) override;
#endif
	// ~End UObject interface.

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
	 * @param Barrier The AGX Dynamics shape to copy from.
	 */
	virtual void CopyFrom(const FShapeBarrier& Barrier, FAGX_ImportContext* Context);

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
	FVector LocalLocation;
	FQuat LocalRotation;

	// Determine if this Shape Component is part of a Rigid Body Component or not. If it is, then
	// the native should be given a local transform that is relative to that Rigid Body, including
	// any intermediate Components. If it is not then the native should be given a local transform
	// that is the global transform of the Shape. We assume that if there is no parent Rigid Body
	// Component then the Shape is free-floating and the native's frame hierarchy does not have any
	// parent, i.e. the local transform is the same as the global transform. This assumption will
	// not hold once we start supporting AGX Dynamics' Assembly.
	UAGX_RigidBodyComponent* Body =
		FAGX_ObjectUtilities::FindFirstAncestorOfType<UAGX_RigidBodyComponent>(*this);
	if (Body != nullptr)
	{
		const FTransform& BodyTransform = Body->GetComponentTransform();
		const FTransform& ShapeTransform = GetComponentTransform();
		const FTransform RelativeTransform = ShapeTransform.GetRelativeTransform(BodyTransform);
		LocalLocation = RelativeTransform.GetLocation();
		LocalRotation = RelativeTransform.GetRotation();
	}
	else
	{
		// Here we assume that any AGX Dynamics Geometry that is not part of a Rigid Body isn't
		// part of anything else either, and thus the local and global transforms are the same,
		// which means that the Shape Component's world transform can be written to the AGX Dynamics
		// Geometry's local transform. Something else will be required once we are to support AGX
		// Dynamics' Assembly.
		LocalLocation = GetComponentTransform().GetLocation();
		LocalRotation = GetComponentTransform().GetRotation();
	}

	Native.SetLocalPosition(LocalLocation);
	Native.SetLocalRotation(LocalRotation);
}
