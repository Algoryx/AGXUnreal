// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_MotionControl.h"
#include "MassPropertiesBarrier.h"

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Containers/UnrealString.h"
#include "Math/Vector.h"
#include "Math/Quat.h"

// Standard library includes.
#include <memory>

#include "RigidBodyBarrier.generated.h"

struct FRigidBodyRef;

struct FAnyShapeBarrier;
struct FBoxShapeBarrier;
struct FCapsuleShapeBarrier;
struct FCylinderShapeBarrier;
class FMassPropertiesBarrier;
struct FShapeBarrier;
struct FSphereShapeBarrier;
struct FTrimeshShapeBarrier;
struct FMergeSplitPropertiesBarrier;

/**
 * Barrier between UAGX_RigidBody and agx::RigidBody. UAGX_RigidBody holds an
 * instance of RigidBodyBarrier and hidden behind the RigidBodyBarrier is a
 * agx::RigidBodyRef. This allows UAGX_RigidBody to interact with
 * agx::RigidBody without including agx/RigidBody.h.
 *
 * This class handles all translation between Unreal Engine types and
 * AGX Dynamics types, such as back and forth between FVector and agx::Vec3.
 */
USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FRigidBodyBarrier
{
	GENERATED_BODY()

	FRigidBodyBarrier();
	FRigidBodyBarrier(std::shared_ptr<FRigidBodyRef> Native);

	void SetEnabled(bool Enabled);
	bool GetEnabled() const;

	void SetPosition(const FVector& Position);
	FVector GetPosition() const;

	void SetRotation(const FQuat& Rotation);
	FQuat GetRotation() const;

	void SetVelocity(const FVector& Velocity);
	FVector GetVelocity() const;

	// In degrees/s.
	void SetAngularVelocity(const FVector& AngularVelocity);

	// In degrees/s.
	FVector GetAngularVelocity() const;

	void SetLinearVelocityDamping(const FVector& LinearVelocityDamping);
	FVector GetLinearVelocityDamping() const;

	void SetAngularVelocityDamping(const FVector& AngularVelocityDamping);
	FVector GetAngularVelocityDamping() const;

	FMassPropertiesBarrier& GetMassProperties();
	const FMassPropertiesBarrier& GetMassProperties() const;
	void UpdateMassProperties();

	double CalculateMass() const;

	void SetCenterOfMassOffset(const FVector& Offset);
	FVector GetCenterOfMassOffset() const;

	FVector GetCenterOfMassPosition() const;

	void SetName(const FString& NewName);
	FString GetName() const;

	void SetMotionControl(EAGX_MotionControl MotionControl);
	EAGX_MotionControl GetMotionControl() const;

	FGuid GetGuid() const;

	void AddShape(FShapeBarrier* Shape);
	void RemoveShape(FShapeBarrier* Shape);


	void AddForceAtCenterOfMass(const FVector& Force);
	void AddForceAtLocalLocation(const FVector& Force, const FVector& Location);
	void AddForceAtWorldLocation(const FVector& Force, const FVector& Location);
	FVector GetForce() const;

	void AddTorqueWorld(const FVector& Torque);
	void AddTorqueLocal(const FVector& Torque);
	FVector GetTorque() const;

	FMergeSplitPropertiesBarrier GetMergeSplitProperties() const;

	bool IsAutomaticallyMerged();
	bool Split();

	void MoveTo(const FVector& Position, const FQuat& Rotation, double Duration);

	bool HasNative() const;
	void AllocateNative();
	FRigidBodyRef* GetNative();
	const FRigidBodyRef* GetNative() const;

	/// @return The address of the underlying AGX Dynamics object.
	uintptr_t GetNativeAddress() const;

	/// Re-assign this Barrier to the given native address. The address must be an existing AGX
	/// Dynamics object of the correct type.
	void SetNativeAddress(uintptr_t NativeAddress);

	void ReleaseNative();

	TArray<FAnyShapeBarrier> GetShapes() const;
	TArray<FSphereShapeBarrier> GetSphereShapes() const;
	TArray<FBoxShapeBarrier> GetBoxShapes() const;
	TArray<FCylinderShapeBarrier> GetCylinderShapes() const;
	TArray<FCapsuleShapeBarrier> GetCapsuleShapes() const;
	TArray<FTrimeshShapeBarrier> GetTrimeshShapes() const;

private:
	std::shared_ptr<FRigidBodyRef> NativeRef;
	FMassPropertiesBarrier MassProperties;
};
