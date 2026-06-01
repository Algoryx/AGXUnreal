// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Math/Vector.h"

// Standard library includes.
#include <memory>

struct FMassPropertiesPtr;
struct FRigidBodyRef;

/**
 * MassProperties is a helper class that provide mass-related functionality to rigid bodies.
 *
 * A MassProperty instance is always tightly coupled with an owning RigidBody instance. Instances of
 * FMassPropertiesBarrier should therefore not be created directly, but instead accessed through
 * a FRigidBodyBarrier's GetMassProperties member function.
 */
class AGXUNREALBARRIER_API FMassPropertiesBarrier
{
public:
	FMassPropertiesBarrier();
	FMassPropertiesBarrier(std::shared_ptr<FMassPropertiesPtr> Native);

	/**
	 * @param NewMass Mass of the rigid body, in kg.
	 */
	void SetMass(float NewMass);
	/**
	 * @return Mass of the rigid body, in kg.
	 */
	float GetMass() const;

	/**
	 * Set the diagonal of the body's inertia matrix.
	 *
	 * @param NewPrincipalInertia
	 */
	void SetPrincipalInertia(const FVector& NewPrincipalInertia);
	/**
	 * @return Principal Inertia of the rigid body as an FVector.
	 */
	FVector GetPrincipalInertia() const;

	void SetAutoGenerateMass(bool bAuto);
	bool GetAutoGenerateMass() const;

	void SetAutoGenerateCenterOfMassOffset(bool bAuto);
	bool GetAutoGenerateCenterOfMassOffset() const;

	void SetAutoGeneratePrincipalInertia(bool bAuto);
	bool GetAutoGeneratePrincipalInertia() const;

	bool HasNative() const;
	FMassPropertiesPtr* GetNative();
	const FMassPropertiesPtr* GetNative() const;
	// No AllocateNative or ReleaseNative because MassProperties are not stand-alone objects in AGX
	// Dynamics, the are always owned by a RigidBody;

	/**
	 * Bind this MassProperties to the native MassProperties object owned by the given RigidBody.
	 *
	 * The given body must already have a native AGX Dynamics body.
	 *
	 * It is rarely necessary to call this member function anywhere outside of FRigidBodyBarrier.
	 * Getting a pre-bound FMassPropertiesBarrier from FRigidBodyBarrier::GetMassProperties is the
	 * recommended way.
	 *
	 * @param RigidBody The body for which this MassProperties should operate.
	 */
	void BindTo(FRigidBodyRef& RigidBody);

private:
	std::shared_ptr<FMassPropertiesPtr> NativePtr;
};
