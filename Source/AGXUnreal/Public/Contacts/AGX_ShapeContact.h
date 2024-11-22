// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Contacts/AGX_ContactPoint.h"
#include "Contacts/ShapeContactBarrier.h"
#include "RigidBodyBarrier.h"
#include "Shapes/ShapeBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "AGX_ShapeContact.generated.h"

class UAGX_ShapeComponent;
class UAGX_RigidBodyComponent;

/**
 * Struct that holds contact data for a shape. Note that this data is only valid during a single
 * simulation step.
 */
USTRUCT(Category = "AGX", BlueprintType)
struct AGXUNREAL_API FAGX_ShapeContact
{
	GENERATED_BODY()

	FAGX_ShapeContact() = default;
	explicit FAGX_ShapeContact(const FShapeContactBarrier& InBarrier);
	explicit FAGX_ShapeContact(FShapeContactBarrier&& InBarrier);

	/**
	 * @return True if this ShapeContact is backed by native AGX Dynamics data.
	 */
	bool HasNative() const;

	/**
	 * @return True if the Shape Contact is enabled, i.e., its ContactPoints are included in the
	 * upcoming solve.
	 */
	bool IsEnabled() const;

	FRigidBodyBarrier GetBody1() const;
	FRigidBodyBarrier GetBody2() const;

	FShapeBarrier GetShape1() const;
	FShapeBarrier GetShape2() const;

	bool Contains(const FRigidBodyBarrier& Body) const;
	bool Contains(const FShapeBarrier& Shape) const;

	int32 IndexOf(const FRigidBodyBarrier& Body) const;
	int32 IndexOf(const FShapeBarrier& Shape) const;

	/**
	 * @param PointIndex The index of the contact point to calculate the relative velocity at.
	 * @return The relative velocity at the contact point, in the world coordinate system.
	 */
	FVector CalculateRelativeVelocity(int32 PointIndex) const;

	int32 GetNumContactPoints() const;

	TArray<FAGX_ContactPoint> GetContactPoints() const;

	FAGX_ContactPoint GetContactPoint(int32 Index) const;

private:
	FShapeContactBarrier Barrier;
};

/**
 * This class acts as an API that exposes functions of FAGX_ShapeContact in Blueprints.
 */
UCLASS()
class AGXUNREAL_API UAGX_ShapeContact_FL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/**
	 * Determine if the Shape Contact represents a valid AGX Dynamics contact or not. No functions
	 * should be called on it if there is no native AGX Dynamics Shape Contact.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contact")
	static bool HasNative(UPARAM(Ref) FAGX_ShapeContact& ShapeContact);

	/**
	 * Determine if the Shape Contact is enabled, i.e. whether the Contact Points it contains should
	 * be seen by the solver.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contact")
	static bool IsEnabled(UPARAM(Ref) FAGX_ShapeContact& ShapeContact);

	/**
	 * Get the first, of two, Shape that collided.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contact")
	static UAGX_ShapeComponent* GetFirstShape(UPARAM(Ref) FAGX_ShapeContact& ShapeContact);

	/**
	 * Get the second, of two, Shape that collided.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contact")
	static UAGX_ShapeComponent* GetSecondShape(UPARAM(Ref) FAGX_ShapeContact& ShapeContact);

	/**
	 * Get the first, of two, Rigid Body that collided.
	 *
	 * Returns None / nullptr if the first Shape does not have a Rigid Body among the attachment
	 * ancestors.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contact")
	static UAGX_RigidBodyComponent* GetFirstBody(UPARAM(Ref) FAGX_ShapeContact& ShapeContact);

	/**
	 * Get the second, of two, Rigid Body that collided.
	 *
	 * Returns None / nullptr if the second Shape does not have a Rigid Body among the attachment
	 * ancestors.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contact")
	static UAGX_RigidBodyComponent* GetSecondBody(UPARAM(Ref) FAGX_ShapeContact& ShapeContact);

	/**
	 * Determine if the Shape Contact includes the given Rigid Body.
	 */
	UFUNCTION(
		BlueprintCallable, BlueprintPure, Category = "AGX Shape Contact",
		Meta = (DisplayName = "Contains"))
	static bool ContainsRigidBody(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, UAGX_RigidBodyComponent* RigidBody,
		bool& bSuccess);

	/**
	 * Determine if the Shape Contact includes the given Shape.
	 */
	UFUNCTION(
		BlueprintCallable, BlueprintPure, Category = "AGX Shape Contact",
		Meta = (DisplayName = "Contains"))
	static bool ContainsShape(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, UAGX_ShapeComponent* Shape, bool& bSuccess);

	/**
	 * Find the index of the given Rigid Body in the Shape Contact.
	 */
	UFUNCTION(
		BlueprintCallable, BlueprintPure, Category = "AGX Shape Contact",
		Meta = (DisplayName = "Index Of"))
	static int32 IndexOfRigidBody(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, UAGX_RigidBodyComponent* RigidBody,
		bool& bSuccess);

	/**
	 * Find the index of the given Shape in the Shape Contact.
	 */
	UFUNCTION(
		BlueprintCallable, BlueprintPure, Category = "AGX Shape Contact",
		Meta = (DisplayName = "Index Of"))
	static int32 IndexOfShape(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, UAGX_ShapeComponent* Shape, bool& bSuccess);
	/**
	 * Get the number of contact points in the Shape Contacts.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contact")
	static int32 GetNumContactPoints(UPARAM(Ref) FAGX_ShapeContact& ShapeContact);

	/**
	 * Get the number of contact points in the Shape Contacts.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contact")
	static int32 GetNumPoints(UPARAM(Ref) FAGX_ShapeContact& ShapeContact);

	/**
	 * Returns the last valid point index. Returns -1 if there are no points and thus no valid
	 * index.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contact")
	static int32 GetLastPointIndex(UPARAM(Ref) FAGX_ShapeContact& ShapeContact);

	/**
	 * Determine if the contact point represents a valid AGX Dynamics Contact Point or not. No
	 * functions should be called on it if there is not native AGX Dynamics Contact Point.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contact")
	static bool HasPointNative(UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex);

	/**
	 * Enable or disable the Contact Point. Only enabled Contact Points are seen by the solver,
	 * and only if the enabled Contact Point is part of an enabled Shape Contact.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contact")
	static bool SetPointEnabled(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool bEnabled);

	/**
	 * Determine is the Contact Point is enabled, i.e. whether the Contact Point should be seen by
	 * the solver. Only Contact Points that are part of an enabled Shape Contact is seen by the
	 * solver.
	 *
	 * @param bSuccess False if the Shape Contact doesn't have a native, the Point Index is out of
	 * range, or the Contact Point doesn't have a native.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contact")
	static bool IsPointEnabled(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess);

	/**
	 * Set the world location of the contact point [cm].
	 *
	 * The location specifies where the solver will apply contact forces onto the Rigid Bodies
	 * involved in the contact.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contact")
	static bool SetPointLocation(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex, FVector Location);

	/**
	 * Get the world location of the Contact Point [cm].
	 *
	 * The location specifies where the solver will apply contact forces onto the Rigid Bodies
	 * involved in the contact.
	 *
	 * @param bSuccess False if the Shape Contact doesn't have a native, the Point Index is out of
	 * range, or the Contact Point doesn't have a native.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contact")
	static FVector GetPointLocation(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess);

	/**
	 * Set the normal direction of the Contact Point. Should be a unit, i.e. length=1.0, vector.
	 *
	 * The normal specifies the direction in which the solver will apply normal forces from the
	 * Contact Point.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contact")
	static bool SetPointNormal(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex, FVector Normal);

	/**
	 * Get the normal direction of the Contact Point.
	 *
	 * The normal specifies the direction in which the solver will apply normal forces from the
	 * contact point.
	 *
	 * @param bSuccess False if the Shape Contact doesn't have a native, the Point Index is out of
	 * range, or the Contact Point doesn't have a native.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contact")
	static FVector GetPointNormal(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess);

	/**
	 * Set the first, of two, tangent direction of the Contact Point.
	 *
	 * The tangent specifies the direction in which the solver will apply friction forces from the
	 * Contact Point.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contact")
	static bool SetPointTangentU(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex, FVector TangentU);

	/**
	 * Get the first, of two, tangent direction of the Contact Point.
	 *
	 * The tangent specifies the direction in which the solver will apply friction forces from the
	 * Contact Point.
	 *
	 * @param bSuccess False if the Shape Contact doesn't have a native, the Point Index is out of
	 * range, or the Contact Point doesn't have a native.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contact")
	static FVector GetPointTangentU(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess);

	/**
	 * Set the second, of two, tangent direction of the Contact Point.
	 *
	 * The tangent specifies the direction in which the solver will apply friction forces from the
	 * Contact Point.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contact")
	static bool SetPointTangentV(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex, FVector TangentV);

	/**
	 * Get the second, of two, tangent direction of the Contact Point.
	 *
	 * The tangent specifies the direction in which the solver will apply friction forces from the
	 * Contact Point.
	 *
	 * @param bSuccess False if the Shape Contact doesn't have a native, the Point Index is out of
	 * range, or the Contact Point doesn't have a native.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contact")
	static FVector GetPointTangentV(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess);

	/**
	 * Set the penetration depth of this contact point [cm].
	 *
	 * A larger depth will result in larger contact
	 * forces.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contact")
	static bool SetPointDepth(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex, double Depth);

	/**
	 * Get the intersection depth between the two colliding Shapes [cm].
	 *
	 * @param bSuccess False if the Shape Contact doesn't have a native, the Point Index is out of
	 * range, or the Contact Point doesn't have a native.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contact")
	static double GetPointDepth(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess);

	/**
	 * Get the witness point for either of the Shapes involved in the contact. The witness point is
	 * a point on the surface of the Shape in the direction of the normal from the contact point
	 * location.
	 *
	 * Shape Index should be either 0 or 1.
	 *
	 * The location is in world coordinates and at time of collision detection. This means that
	 * after the solve / step the geometries will most likely have moved.
	 *
	 * @param PointIndex The index of the Contact Point in the Shape Contact to inspect.
	 * @param ShapeIndex The index of the Shape in the Shape Contact to get witness point for. 0
	 * or 1.
	 * @param bSuccess False if the Shape Contact doesn't have a native, the Point Index is out of
	 * range, or the Contact Point doesn't have a native.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX SHapeContacts")
	static FVector GetPointWitnessPoint(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex, int32 ShapeIndex,
		bool& bSuccess);

	/**
	 * Set the area covered by this contact point.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contact")
	static bool SetPointArea(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex, double Area);

	/**
	 * Get the area represented by the contact point.
	 *
	 * Only relevant if Use Contact Area Approach is enabled on the Contact Material.
	 *
	 * @param bSuccess False if the Shape Contact doesn't have a native, the Point Index is out of
	 * range, or the Contact Point doesn't have a native.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contact")
	static double GetPointArea(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess);

	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contact")
	static bool SetPointSurfaceVelocity(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex, FVector SurfaceVelocity);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contact")
	static FVector GetPointSurfaceVelocity(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess);

	/**
	 * Get the total contact force, in world coordinates. Includes both normal and friction forces.
	 *
	 * @param bSuccess False if the Shape Contact doesn't have a native, the Point Index is out of
	 * range, or the Contact Point doesn't have a native.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contact")
	static FVector GetPointForce(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess);

	/**
	 * Get the magnitude of the contact force. Includes both normal and friction forces.
	 *
	 * This is faster than calling Get Force and taking the length of that vector.
	 *
	 * @param bSuccess False if the Shape Contact doesn't have a native, the Point Index is out of
	 * range, or the Contact Point doesn't have a native.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contact")
	static double GetPointForceMagnitude(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess);

	/**
	 * Get the normal force of the contact, in world coordinates.
	 *
	 * @param bSuccess False if the Shape Contact doesn't have a native, the Point Index is out of
	 * range, or the Contact Point doesn't have a native.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contact")
	static FVector GetPointNormalForce(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess);

	/**
	 * Get the magnitude of the normal force.
	 *
	 * This is faster than calling Get Normal Force and taking the length of that vector.
	 *
	 * @param bSuccess False if the Shape Contact doesn't have a native, the Point Index is out of
	 * range, or the Contact Point doesn't have a native.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contact")
	static double GetPointNormalForceMagnitude(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess);

	/**
	 * Get the tangential, i.e. friction, force of the contact.
	 *
	 * @param bSuccess False if the Shape Contact doesn't have a native, the Point Index is out of
	 * range, or the Contact Point doesn't have a native.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contact")
	static FVector GetPointTangentialForce(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess);

	/**
	 * Get the magnitude of the tangential force.
	 *
	 * This is faster than calling Get Tangential Force and taking the length of that vector.
	 *
	 * @param bSuccess False if the Shape Contact doesn't have a native, the Point Index is out of
	 * range, or the Contact Point doesn't have a native.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contact")
	static double GetPointTangentialForceMagnitude(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess);

	/**
	 * Get the magnitude of the tangential force in the U direction.
	 *
	 * @param bSuccess False if the Shape Contact doesn't have a native, the Point Index is out of
	 * range, or the Contact Point doesn't have a native.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contact")
	static double GetPointTangentialForceUMagnitude(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess);

	/**
	 * Get the magnitude of the tangential force in the V direction.
	 *
	 * @param bSuccess False if the Shape Contact doesn't have a native, the Point Index is out of
	 * range, or the Contact Point doesn't have a native.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contact")
	static double GetPointTangentialForceVMagnitude(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess);

	/**
	 * Get all contact force component magnitudes. Ordered normal, tangent U, tangent V.
	 *
	 * @param bSuccess False if the Shape Contact doesn't have a native, the Point Index is out of
	 * range, or the Contact Point doesn't have a native.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contact")
	static FVector GetPointLocalForce(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess);

	/**
	 * Get the magnitude of one component of the contact force.
	 *
	 * @param Component The part of the contact force to get the magnitude for.
	 * @param bSuccess False if the Shape Contact doesn't have a native, the Point Index is out of
	 * range, or the Contact Point doesn't have a native.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "AGX Shape Contact")
	static double GetPointLocalForceComponent(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex,
		EAGX_ContactForceComponents Component, bool& bSuccess);

	/**
	 * Compute the relative velocity at the given contact point. Will be zero for a stable contact.
	 *
	 * @param bSuccess False if the Shape Contact doesn't have a native, the Point Index is out of
	 * range, or the Contact Point doesn't have a native.
	 */
	UFUNCTION(BlueprintCallable, Category = "AGX Shape Contact")
	static FVector CalculateRelativeVelocity(
		UPARAM(Ref) FAGX_ShapeContact& ShapeContact, int32 PointIndex, bool& bSuccess);
};
