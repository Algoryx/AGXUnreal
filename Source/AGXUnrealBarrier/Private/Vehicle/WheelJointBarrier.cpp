// Copyright 2025, Algoryx Simulation AB.

#include "Vehicle/WheelJointBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/AGXTypeConversions.h"
#include "Constraints/ControllerConstraintBarriers.h"
#include "RigidBodyBarrier.h"
#include "Utilities/AGX_BarrierConstraintUtilities.h"

#include "BeginAGXIncludes.h"
#include <agxVehicle/WheelJoint.h>
#include "EndAGXIncludes.h"

FWheelJointBarrier::FWheelJointBarrier()
	: FConstraintBarrier()
{
}

FWheelJointBarrier::FWheelJointBarrier(std::unique_ptr<FConstraintRef> Native)
	: FConstraintBarrier(std::move(Native))
{
	check(NativeRef->Native->is<agxVehicle::WheelJoint>());
}

FWheelJointBarrier::~FWheelJointBarrier()
{
}

namespace WheelJointBarrier_helpers
{
	template <typename Barrier>
	TUniquePtr<Barrier> CreateControllerBarrier(agx::BasicControllerConstraint* Controller)
	{
		return TUniquePtr<Barrier>(
			new Barrier(std::make_unique<FConstraintControllerRef>(Controller)));
	}

	agxVehicle::WheelJoint* GetNativeWheelJoint(const FConstraintRef& NativeRef)
	{
		return dynamic_cast<agxVehicle::WheelJoint*>(NativeRef.Native.get());
	}
}

double FWheelJointBarrier::GetAngle() const
{
	check(HasNative());
	return ConvertAngleToUnreal<double>(
		WheelJointBarrier_helpers::GetNativeWheelJoint(*NativeRef)->getAngle());
}

TUniquePtr<FLockControllerBarrier> FWheelJointBarrier::GetLockController(
	EAGX_WheelJointSecondaryConstraint Sc)
{
	check(HasNative());
	using namespace WheelJointBarrier_helpers;
	const auto ScAGX = Convert(Sc);
	return CreateControllerBarrier<FLockControllerBarrier>(
		GetNativeWheelJoint(*NativeRef)->getLock1D(ScAGX));
}

TUniquePtr<FRangeControllerBarrier> FWheelJointBarrier::GetRangeController(
	EAGX_WheelJointSecondaryConstraint Sc)
{
	check(HasNative());
	using namespace WheelJointBarrier_helpers;
	const auto ScAGX = Convert(Sc);
	return CreateControllerBarrier<FRangeControllerBarrier>(
		GetNativeWheelJoint(*NativeRef)->getRange1D(ScAGX));
}

TUniquePtr<FTargetSpeedControllerBarrier> FWheelJointBarrier::GetTargetSpeedController(
	EAGX_WheelJointSecondaryConstraint Sc)
{
	check(HasNative());
	using namespace WheelJointBarrier_helpers;
	const auto ScAGX = Convert(Sc);
	return CreateControllerBarrier<FTargetSpeedControllerBarrier>(
		GetNativeWheelJoint(*NativeRef)->getMotor1D(ScAGX));
}

TUniquePtr<const FLockControllerBarrier> FWheelJointBarrier::GetLockController(
	EAGX_WheelJointSecondaryConstraint Sc) const
{
	check(HasNative());
	using namespace WheelJointBarrier_helpers;
	const auto ScAGX = Convert(Sc);
	return CreateControllerBarrier<const FLockControllerBarrier>(
		GetNativeWheelJoint(*NativeRef)->getLock1D(ScAGX));
}

TUniquePtr<const FRangeControllerBarrier> FWheelJointBarrier::GetRangeController(
	EAGX_WheelJointSecondaryConstraint Sc) const
{
	check(HasNative());
	using namespace WheelJointBarrier_helpers;
	const auto ScAGX = Convert(Sc);
	return CreateControllerBarrier<const FRangeControllerBarrier>(
		GetNativeWheelJoint(*NativeRef)->getRange1D(ScAGX));
}

TUniquePtr<const FTargetSpeedControllerBarrier> FWheelJointBarrier::GetTargetSpeedController(
	EAGX_WheelJointSecondaryConstraint Sc) const
{
	check(HasNative());
	using namespace WheelJointBarrier_helpers;
	const auto ScAGX = Convert(Sc);
	return CreateControllerBarrier<FTargetSpeedControllerBarrier>(
		GetNativeWheelJoint(*NativeRef)->getMotor1D(ScAGX));
}

void FWheelJointBarrier::AllocateNativeImpl(
	const FRigidBodyBarrier& RigidBody1, const FVector& FramePosition1, const FQuat& FrameRotation1,
	const FRigidBodyBarrier* RigidBody2, const FVector& FramePosition2, const FQuat& FrameRotation2)
{
	check(!HasNative());

	agx::RigidBody* NativeRigidBody1 = nullptr;
	agx::RigidBody* NativeRigidBody2 = nullptr;
	agx::FrameRef NativeFrame1 = nullptr;
	agx::FrameRef NativeFrame2 = nullptr;

	FAGX_BarrierConstraintUtilities::ConvertConstraintBodiesAndFrames(
		RigidBody1, FramePosition1, FrameRotation1, RigidBody2, FramePosition2, FrameRotation2,
		NativeRigidBody1, NativeFrame1, NativeRigidBody2, NativeFrame2);

	NativeRef->Native = new agxVehicle::WheelJoint(
		NativeRigidBody1, NativeFrame1.get(), NativeRigidBody2, NativeFrame2.get());
}
