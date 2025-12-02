// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Contacts/AGX_ContactEnums.h"
#include "Contacts/ContactPointBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ContactPoint.generated.h"

USTRUCT(Category = "AGX", BlueprintType)
struct AGXUNREAL_API FAGX_ContactPoint
{
	GENERATED_BODY()

public:
	FAGX_ContactPoint() = default;
	FAGX_ContactPoint(const FAGX_ContactPoint& InOther);
	FAGX_ContactPoint(FContactPointBarrier&& InBarrier);
	FAGX_ContactPoint& operator=(const FAGX_ContactPoint& InOther);

	bool HasNative() const;

	//
	// Collision detection state.
	//

	/**
	 * Enable or disable this contact point.
	 */
	void SetEnabled(bool bEnable);

	/**
	 * @return True if this contact point participates in the upcoming solve.
	 */
	bool IsEnabled() const;

	/**
	 * Set the world space location of the contact point [cm].
	 */
	void SetLocation(const FVector& Location);

	/**
	 * @return The location of the contact point, in world coordinates [cm].
	 */
	FVector GetLocation() const;

	/**
	 * Set the normal of the contact point, i.e. the direction in which contact forces will be
	 * applied.
	 */
	void SetNormal(const FVector& Normal);

	/**
	 * @return The normal of the contact point, in world coordinates.
	 */
	FVector GetNormal() const;

	/**
	 * Set the U tangent direction of the contact point, which is also a friction direction.
	 *
	 * The normal direction and the two tangent directions should form an orthonormal coordinate
	 * system.
	 */
	void SetTangentU(const FVector& TangentU);

	/**
	 * @return The U tangent of the contact point, in world coordinates.
	 */
	FVector GetTangentU() const;

	/**
	 * Set the V tangent direction of the contact point, which is also a friction direction.
	 *
	 * The normal direction and the two tangent directions should form an orthonormal coordinate
	 * system.
	 */
	void SetTangentV(const FVector& TangentV);

	/**
	 * @return The V tangent of the contact point, in world coordinates.
	 */
	FVector GetTangentV() const;

	/**
	 * Set the penetration depth of this contact point [cm].
	 *
	 * A larger depth will result in larger contact
	 * forces.
	 */
	void SetDepth(double Depth);

	/**
	 * @return The depth of the overlap [cm].
	 */
	double GetDepth() const;

	/**
	 * Get witness point location for the ith (0 or 1) Shape [cm].
	 *
	 * The location is in world coordinates and at time of collision detection. This means that
	 * after the solve/step the geometries will most likely have moved.
	 *
	 * @param Index 0 for first Shape, 1 for second.
	 * @return witness point for the ith Shape, in world coordinates at collision detection time.
	 */
	FVector GetWitnessPoint(int32 Index) const;

	/**
	 * Set the area covered by this contact point [cm^2].
	 *
	 * Only used if the contact material has contact area enabled.
	 */
	void SetArea(double Area) const;

	/**
	 * Get the area covered by this contact point [cm^2].
	 *
	 * Only non-zero when the contact material has contact area enabled.
	 * @return The estimated area.
	 */
	double GetArea() const;

	/**
	 * Set the surface velocity at the contact point [cm/s].
	 *
	 * Surface velocity causes the solver to add a relative velocity at the contact point.
	 */
	void SetSurfaceVelocity(const FVector& SurfaceVelocity);

	/**
	 * Get the surface velocity at the contact point [cm/s].
	 *
	 * Surface velocity causes the solver to add a relative velocity at the contact point.
	 */
	FVector GetSurfaceVelocity() const;

	//
	// Solver state getters. May only be called after the solver.
	//

	/**
	 * @return The complete (signed) contact force, including both normal and tangential (friction),
	 * in the world coordinate system [N].
	 */
	FVector GetForce() const;

	/**
	 * Get the magnitude of the contact force [N].
	 *
	 * Includes both normal and friction forces.
	 * This is faster than calling Get Force and taking the length of that vector.
	 */
	double GetForceMagnitude() const;

	/**
	 * @return The (signed) normal force in world coordinates [N].
	 */
	FVector GetNormalForce() const;

	/**
	 * Get the magnitude of the normal force [N].
	 *
	 * This is faster than calling Get Normal Force and taking the length of that vector.
	 */
	double GetNormalForceMagnitude() const;

	/**
	 * @return The (signed) tangential force (friction force) in world coordinates [N].
	 */
	FVector GetTangentialForce() const;

	/**
	 * Get the magnitude of the tangential force [N].
	 *
	 * This is faster than calling Get Tangential Force and taking the length of that vector.
	 */
	double GetTangentialForceMagnitude() const;

	/**
	 * Get the magnitude of the tangential force in the U direction [N].
	 */
	double GetTangentialForceUMagnitude() const;

	/**
	 * Get the magnitude of the tangential force in the V direction [N].
	 */
	double GetTangentialForceVMagnitude() const;

	/**
	 * Get the contact force in the contact's local coordinate system [N].
	 *
	 * In the order normal, tangent U, tangent V.
	 */
	FVector GetLocalForce() const;

	/**
	 * Get the magnitude of one component of the contact force [N].
	 * @param Index 0=Normal, 1=Tangent U, 2=Tangent V.
	 */
	double GetLocalForce(EAGX_ContactForceComponents Component);

private:
	FContactPointBarrier Barrier;
};
