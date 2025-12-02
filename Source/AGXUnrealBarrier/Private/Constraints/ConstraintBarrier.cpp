// Copyright 2025, Algoryx Simulation AB.

#include "Constraints/ConstraintBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGXBarrierFactories.h"
#include "AGX_AgxDynamicsObjectsAccess.h"
#include "BarrierOnly/AGXRefs.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/LockJoint.h>
#include <agx/Prismatic.h>
#include "EndAGXIncludes.h"

// Unreal Engine includes.
#include <Misc/AssertionMacros.h>

FConstraintBarrier::FConstraintBarrier()
	: NativeRef {new FConstraintRef()}
{
}

FConstraintBarrier::FConstraintBarrier(FConstraintBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
	Other.NativeRef.reset(new FConstraintRef());
}

FConstraintBarrier::FConstraintBarrier(std::unique_ptr<FConstraintRef> Native)
	: NativeRef(std::move(Native))
{
}

FConstraintBarrier::~FConstraintBarrier()
{
}

bool FConstraintBarrier::HasNative() const
{
	return NativeRef && NativeRef->Native;
}

FConstraintRef* FConstraintBarrier::GetNative()
{
	return NativeRef.get();
}

const FConstraintRef* FConstraintBarrier::GetNative() const
{
	return NativeRef.get();
}

void FConstraintBarrier::AllocateNative(
	const FRigidBodyBarrier& RigidBody1, const FVector& FramePosition1, const FQuat& FrameRotation1,
	const FRigidBodyBarrier* RigidBody2, const FVector& FramePosition2, const FQuat& FrameRotation2)
{
	check(!HasNative());

	AllocateNativeImpl(
		RigidBody1, FramePosition1, FrameRotation1, RigidBody2, FramePosition2, FrameRotation2);
}

void FConstraintBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->Native = nullptr;
}

bool FConstraintBarrier::GetValid() const
{
	check(HasNative());
	return NativeRef->Native->getValid();
}

void FConstraintBarrier::SetName(const FString& NameUnreal)
{
	check(HasNative());
	agx::String NameAGX = Convert(NameUnreal);
	NativeRef->Native->setName(NameAGX);
}

FString FConstraintBarrier::GetName() const
{
	check(HasNative());
	FString NameUnreal(Convert(NativeRef->Native->getName()));
	return NameUnreal;
}

void FConstraintBarrier::SetEnable(bool Enable)
{
	check(HasNative());
	NativeRef->Native->setEnable(Enable);
}

bool FConstraintBarrier::GetEnable() const
{
	check(HasNative());
	return NativeRef->Native->getEnable();
}

void FConstraintBarrier::SetSolveType(int32 SolveType)
{
	check(HasNative());
	NativeRef->Native->setSolveType(agx::Constraint::SolveType(SolveType));
}

int32 FConstraintBarrier::GetSolveType() const
{
	check(HasNative());
	return int32(NativeRef->Native->getSolveType());
}

void FConstraintBarrier::SetElasticity(double Elasticity, int32 Dof)
{
	check(HasNative());
	NativeRef->Native->setElasticity(Elasticity, Dof);
}

double FConstraintBarrier::GetElasticity(int32 Dof) const
{
	check(HasNative());
	return NativeRef->Native->getElasticity(Dof);
}

void FConstraintBarrier::SetCompliance(double Compliance, int32 Dof)
{
	check(HasNative());
	NativeRef->Native->setCompliance(Compliance, Dof);
}

double FConstraintBarrier::GetCompliance(int32 Dof) const
{
	check(HasNative());
	return NativeRef->Native->getCompliance(Dof);
}

void FConstraintBarrier::SetSpookDamping(double SpookDamping, int32 Dof)
{
	check(HasNative());
	NativeRef->Native->setDamping(SpookDamping, Dof);
}

double FConstraintBarrier::GetSpookDamping(int32 Dof) const
{
	check(HasNative());
	return NativeRef->Native->getDamping(Dof);
}

void FConstraintBarrier::SetForceRange(double Min, double Max, int32 Dof)
{
	check(HasNative());
	return NativeRef->Native->setForceRange(agx::RangeReal(Min, Max), Dof);
}

void FConstraintBarrier::GetForceRange(double* Min, double* Max, int32 Dof) const
{
	check(HasNative());
	agx::RangeReal Range = NativeRef->Native->getForceRange(Dof);

	if (Min)
		*Min = Range.lower();

	if (Max)
		*Max = Range.upper();
}

FAGX_RealInterval FConstraintBarrier::GetForceRange(int32 Dof) const
{
	check(HasNative());
	agx::RangeReal Range = NativeRef->Native->getForceRange(Dof);

	return FAGX_RealInterval(Range.lower(), Range.upper());
}

double FConstraintBarrier::GetCurrentForce(int32 Dof)
{
	check(HasNative());
	return NativeRef->Native->getCurrentForce(Dof);
}


void FConstraintBarrier::SetEnableComputeForces(bool bEnable)
{
	check(HasNative());
	NativeRef->Native->setEnableComputeForces(bEnable);
}

bool FConstraintBarrier::GetEnableComputeForces() const
{
	check(HasNative());
	return NativeRef->Native->getEnableComputeForces();
}

namespace ConstraintBarrier_helpers
{
	struct FLastForce
	{
		agx::Vec3 ForceAGX;
		agx::Vec3 TorqueAGX;

		FVector& OutForce;
		FVector& OutTorque;

		FLastForce(FVector& InForce, FVector& InTorque)
			: OutForce(InForce)
			, OutTorque(InTorque)
		{
		}

		void Write(bool bGotForces)
		{
			if (bGotForces)
			{
				OutForce = ConvertVector(ForceAGX);
				OutTorque = ConvertTorque(TorqueAGX);
			}
			else
			{
				OutForce = FVector::ZeroVector;
				OutTorque = FVector::ZeroVector;
			}
		}
	};
}

bool FConstraintBarrier::GetLastForce(
	int32 BodyIndex, FVector& OutForce, FVector& OutTorque, bool bForceAtCm)
{
	check(HasNative());
	ConstraintBarrier_helpers::FLastForce LastForce(OutForce, OutTorque);
	const bool bGotForces = NativeRef->Native->getLastForce(
		BodyIndex, LastForce.ForceAGX, LastForce.TorqueAGX, bForceAtCm);
	LastForce.Write(bGotForces);
	return bGotForces;
}

bool FConstraintBarrier::GetLastForce(
	const FRigidBodyBarrier* Body, FVector& OutForce, FVector& OutTorque, bool bForceAtCm)
{
	check(HasNative());
	ConstraintBarrier_helpers::FLastForce LastForce(OutForce, OutTorque);
	const agx::RigidBody* BodyAGX = FAGX_AgxDynamicsObjectsAccess::TryGetFrom(Body);
	const bool bGotForces = NativeRef->Native->getLastForce(
		BodyAGX, LastForce.ForceAGX, LastForce.TorqueAGX, bForceAtCm);
	LastForce.Write(bGotForces);
	return bGotForces;
}

bool FConstraintBarrier::GetLastLocalForce(
	int32 BodyIndex, FVector& OutForce, FVector& OutTorque, bool bForceAtCm)
{
	check(HasNative());
	ConstraintBarrier_helpers::FLastForce LastForce(OutForce, OutTorque);
	const bool bGotForces = NativeRef->Native->getLastLocalForce(
		BodyIndex, LastForce.ForceAGX, LastForce.TorqueAGX, bForceAtCm);
	LastForce.Write(bGotForces);
	return bGotForces;
}

bool FConstraintBarrier::GetLastLocalForce(
	const FRigidBodyBarrier* Body, FVector& OutForce, FVector& OutTorque, bool bForceAtCm)
{
	check(HasNative());
	ConstraintBarrier_helpers::FLastForce LastForce(OutForce, OutTorque);
	const agx::RigidBody* BodyAGX = FAGX_AgxDynamicsObjectsAccess::TryGetFrom(Body);
	const bool bGotForces = NativeRef->Native->getLastLocalForce(
		BodyAGX, LastForce.ForceAGX, LastForce.TorqueAGX, bForceAtCm);
	LastForce.Write(bGotForces);
	return bGotForces;
}

FGuid FConstraintBarrier::GetGuid() const
{
	check(HasNative());
	FGuid Guid = Convert(NativeRef->Native->getUuid());
	return Guid;
}

bool FConstraintBarrier::HasFirstBody() const
{
	check(HasNative());
	return NativeRef->Native->getNumBodies() >= 1;
}

bool FConstraintBarrier::HasSecondBody() const
{
	check(HasNative());
	return NativeRef->Native->getNumBodies() >= 2;
}

namespace
{
	FRigidBodyBarrier GetBodyAsBarrier(agx::Constraint* Constraint, agx::UInt Index)
	{
		if (Index >= Constraint->getNumBodies())
		{
			return FRigidBodyBarrier();
		}
		return AGXBarrierFactories::CreateRigidBodyBarrier(Constraint->getBodyAt(Index));
	}
}

FRigidBodyBarrier FConstraintBarrier::GetFirstBody() const
{
	check(HasNative());
	return GetBodyAsBarrier(NativeRef->Native, agx::UInt(0));
}

FRigidBodyBarrier FConstraintBarrier::GetSecondBody() const
{
	check(HasNative());
	return GetBodyAsBarrier(NativeRef->Native, agx::UInt(1));
}

namespace
{
	agx::Frame* GetFrame(const agx::Constraint& Native, int32 IndexUnreal)
	{
		check(IndexUnreal >= 0 && IndexUnreal < 2);
		agx::UInt IndexAGX = static_cast<agx::UInt>(IndexUnreal);
		return Native.getAttachment(IndexAGX)->getFrame();
	}
}

void FConstraintBarrier::SetLocalLocation(int32 BodyIndex, const FVector& LocalLocation)
{
	check(HasNative());
	const agx::Vec3 LocalLocationAGX = ConvertDisplacement(LocalLocation);
	GetFrame(*NativeRef->Native, BodyIndex)->setLocalTranslate(LocalLocationAGX);
}

void FConstraintBarrier::SetLocalRotation(int32 BodyIndex, const FQuat& LocalRotation)
{
	check(HasNative());
	const agx::Quat LocalRotationAGX = Convert(LocalRotation);
	GetFrame(*NativeRef->Native, BodyIndex)->setLocalRotate(LocalRotationAGX);
}

FVector FConstraintBarrier::GetLocalLocation(int32 Index) const
{
	check(HasNative());
	agx::Vec3 TranslateAGX = GetFrame(*NativeRef->Native, Index)->getLocalTranslate();
	FVector TranslateUnreal = ConvertDisplacement(TranslateAGX);
	return TranslateUnreal;
}

FQuat FConstraintBarrier::GetLocalRotation(int32 Index) const
{
	check(HasNative());
	agx::Quat RotateAGX = GetFrame(*NativeRef->Native, Index)->getLocalRotate();
	FQuat RotateUnreal = Convert(RotateAGX);
	return RotateUnreal;
}

uintptr_t FConstraintBarrier::GetNativeAddress() const
{
	if (!HasNative())
	{
		return 0;
	}

	return reinterpret_cast<uintptr_t>(NativeRef->Native.get());
}

void FConstraintBarrier::SetNativeAddress(uintptr_t NativeAddress)
{
	if (NativeAddress == GetNativeAddress())
	{
		return;
	}

	if (HasNative())
	{
		ReleaseNative();
	}

	if (NativeAddress == 0)
	{
		NativeRef->Native = nullptr;
		return;
	}

	NativeRef->Native = reinterpret_cast<agx::Constraint*>(NativeAddress);
}

bool FConstraintBarrier::IsRotational() const
{
	check(HasNative());
	// Not bulletproof, but the best way I know.
	if (dynamic_cast<agx::LockJoint*>(NativeRef->Native.get()) != nullptr)
		return false;

	if (dynamic_cast<agx::Prismatic*>(NativeRef->Native.get()) != nullptr)
		return false;

	return true;
}
