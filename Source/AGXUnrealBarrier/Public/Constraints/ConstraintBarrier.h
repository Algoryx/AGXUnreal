// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_RealInterval.h"
#include "RigidBodyBarrier.h"

// Unreal Engine includes.
#include "CoreTypes.h"
#include "Containers/UnrealString.h"
#include "Math/Vector.h"
#include "Math/Quat.h"
#include "Misc/EngineVersionComparison.h"

// Standard library includes.
#include <memory>

struct FConstraintRef;

/**
 * Acts as an interface to a native AGX constraint, and encapsulates it so that
 * it is completely hidden from code that includes this file.
 *
 * To support specialized native constraint types, a barrier class deriving from
 * this class has to be created for each constraint type. The derived class
 * does only need to implement AllocateNativeImpl (see function comment), and
 * supply an interface specific for the specialization. It does not need to hold
 * a reference to the native object. Because the derived class creates the native
 * constraint object in AllocateNativeImpl, it can also safely cast NativeRef->Native
 * to that same type whenever necessary for temporary usage in implementation of
 * its specialized interface.
 */
class AGXUNREALBARRIER_API FConstraintBarrier
{
public:
	FConstraintBarrier();
	FConstraintBarrier(FConstraintBarrier&& Other);
	FConstraintBarrier(std::unique_ptr<FConstraintRef> Native);
	virtual ~FConstraintBarrier();

	bool HasNative() const;
	FConstraintRef* GetNative();
	const FConstraintRef* GetNative() const;

	void AllocateNative(
		const FRigidBodyBarrier& RigidBody1, const FVector& FramePosition1,
		const FQuat& FrameRotation1, const FRigidBodyBarrier* RigidBody2,
		const FVector& FramePosition2, const FQuat& FrameRotation2);

	void ReleaseNative();

	bool GetValid() const;

	void SetEnable(bool Enable);
	bool GetEnable() const;

	void SetName(const FString& Name);
	FString GetName() const;

	void SetSolveType(int32 SolveType);
	int32 GetSolveType() const;

	void SetElasticity(double Elasticity, int32 Dof);
	double GetElasticity(int32 Dof) const;

	void SetCompliance(double Compliance, int32 Dof);
	double GetCompliance(int32 Dof) const;

	void SetSpookDamping(double SpookDamping, int32 Dof);
	double GetSpookDamping(int32 Dof) const;

	void SetForceRange(double Min, double Max, int32 Dof);
	void GetForceRange(double* Min, double* Max, int32 Dof) const;
	FAGX_RealInterval GetForceRange(int32 Dof) const;

	double GetCurrentForce(int32 Dof);

	void SetEnableComputeForces(bool bEnable);
	bool GetEnableComputeForces() const;

	bool GetLastForce(
		int32 BodyIndex, FVector& OutForce, FVector& OutTorque, bool bForceAtCm = false);
	bool GetLastForce(
		const FRigidBodyBarrier* Body, FVector& OutForce, FVector& OutTorque,
		bool bForceAtCm = false);
	bool GetLastLocalForce(
		int32 BodyIndex, FVector& OutForce, FVector& OutTorque, bool bForceAtCm = false);
	bool GetLastLocalForce(
		const FRigidBodyBarrier* Body, FVector& OutForce, FVector& OutTorque,
		bool bForceAtCm = false);

	FGuid GetGuid() const;

	bool HasFirstBody() const;
	bool HasSecondBody() const;

	FRigidBodyBarrier GetFirstBody() const;
	FRigidBodyBarrier GetSecondBody() const;

	/// \todo Consider creating a Barrier for agx::Attachment.
	void SetLocalLocation(int32 BodyIndex, const FVector& LocalLocation);
	void SetLocalRotation(int32 BodyIndex, const FQuat& LocalRotation);

	FVector GetLocalLocation(int32 BodyIndex) const;
	FQuat GetLocalRotation(int32 BodyIndex) const;

	/// @return The address of the underlying AGX Dynamics object.
	uintptr_t GetNativeAddress() const;

	/// Re-assign this Barrier to the given native address. The address must be an existing AGX
	/// Dynamics object of the correct type.
	void SetNativeAddress(uintptr_t NativeAddress);

	bool IsRotational() const;

private:
	FConstraintBarrier(const FConstraintBarrier&) = delete;
	void operator=(const FConstraintBarrier&) = delete;

private:
	/**
	 * Called from AllocateNative. Each subclass must override this function and
	 * within it create the correct agx::Constraint object, and assign it to
	 * NativeRef->Native.
	 *
	 * The override should not call the override of the parent class, to avoid multiple
	 * objects being created in a deeper inheritance tree!
	 *
	 * The derived class should not store a reference to the native object!
	 */
	virtual void AllocateNativeImpl(
		const FRigidBodyBarrier& RigidBody1, const FVector& FramePosition1,
		const FQuat& FrameRotation1, const FRigidBodyBarrier* RigidBody2,
		const FVector& FramePosition2, const FQuat& FrameRotation2) = 0;

protected:
	// NativeRef has equal lifetime as this object ie the FConstraintRef is created at construction.
	// NativeRef->Native can be null.
	// NativeRef->Native is created by the lowermost subclass when AllocateNative is invoked,
	// and released when ReleaseNative is invoked.
	// NativeRef->Native should be type-casted whenever a subclass needs the derived interface (e.g.
	// to agx::LockJoint).
	std::unique_ptr<FConstraintRef> NativeRef;
};
