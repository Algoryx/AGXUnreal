// Copyright 2024, Algoryx Simulation AB.

#include "Constraints/SingleControllerConstraint1DOFBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "AGX_LogCategory.h"
#include "BarrierOnly/AGXRefs.h"
#include "Constraints/ControllerConstraintBarriers.h"
#include "RigidBodyBarrier.h"
#include "Utilities/AGX_BarrierConstraintUtilities.h"

#include "BeginAGXIncludes.h"
#include <agx/SingleControllerConstraint1DOF.h>
#include "EndAGXIncludes.h"

FSingleControllerConstraint1DOFBarrier::FSingleControllerConstraint1DOFBarrier()
	: FConstraint1DOFBarrier()
{
}

FSingleControllerConstraint1DOFBarrier::FSingleControllerConstraint1DOFBarrier(
	std::unique_ptr<FConstraintRef> Native)
	: FConstraint1DOFBarrier(std::move(Native))
{
	/// \todo Should we allow nullptr hinge here?
	check(NativeRef->Native->is<agx::SingleControllerConstraint1DOF>());
}

FSingleControllerConstraint1DOFBarrier::~FSingleControllerConstraint1DOFBarrier()
{
}

void FSingleControllerConstraint1DOFBarrier::AllocateNative(
	const FRigidBodyBarrier& Rb1, const FVector& FramePosition1, const FQuat& FrameRotation1,
	const FRigidBodyBarrier* Rb2, const FVector& FramePosition2, const FQuat& FrameRotation2,
	FConstraintControllerBarrier* Controller, EAGX_ConstraintControllerType ControllerType,
	EAGX_ConstraintAngleControllerType ControllerAngleType)
{
	check(!HasNative());
	check(Controller != nullptr);
	check(!Controller->HasNative());
	check(ControllerType != EAGX_ConstraintControllerType::Invalid);

	// Setup Controller.
	agx::ConstraintAngleBasedData ControllerData = [&]() -> agx::ConstraintAngleBasedData
	{
		if (ControllerAngleType == EAGX_ConstraintAngleControllerType::RotationalAngle)
			return {nullptr, new agx::RotationalAngle(agx::Angle::N)};
		else
			return {nullptr, new agx::SeparationAngle(agx::Angle::N)};
	}();

	switch (ControllerType)
	{
		case EAGX_ConstraintControllerType::ConstraintTargetSpeedController:
			Controller->GetNative()->Native = new agx::TargetSpeedController(ControllerData);
			break;
		case EAGX_ConstraintControllerType::ConstraintLockController:
			Controller->GetNative()->Native = new agx::LockController(ControllerData);
			break;
		case EAGX_ConstraintControllerType::ConstraintRangeController:
			Controller->GetNative()->Native = new agx::RangeController(ControllerData);
			break;
		case EAGX_ConstraintControllerType::ConstraintElectricMotorController:
			Controller->GetNative()->Native = new agx::ElectricMotorController(ControllerData);
			break;
		case EAGX_ConstraintControllerType::ConstraintFrictionController:
			Controller->GetNative()->Native = new agx::FrictionController(ControllerData);
			break;
	}

	if (!Controller->HasNative())
	{
		UE_LOG(
			LogAGX, Error,
			TEXT("Unable to create Controller for FSingleControllerConstraint1DOFBarrier '%s'."),
			*GetName());
		return;
	}

	agx::RigidBody* NativeRigidBody1 = nullptr;
	agx::RigidBody* NativeRigidBody2 = nullptr;
	agx::FrameRef NativeFrame1 = nullptr;
	agx::FrameRef NativeFrame2 = nullptr;

	FAGX_BarrierConstraintUtilities::ConvertConstraintBodiesAndFrames(
		Rb1, FramePosition1, FrameRotation1, Rb2, FramePosition2, FrameRotation2, NativeRigidBody1,
		NativeFrame1, NativeRigidBody2, NativeFrame2);

	NativeRef->Native = new agx::SingleControllerConstraint1DOF(
		NativeRigidBody1, NativeFrame1.get(), NativeRigidBody2, NativeFrame2.get(),
		Controller->GetNative()->Native);
}

EAGX_ConstraintControllerType FSingleControllerConstraint1DOFBarrier::GetControllerType() const
{
	check(HasNative());

	if (auto C = GetTargetSpeedController())
		if (C->HasNative())
			return EAGX_ConstraintControllerType::ConstraintTargetSpeedController;

	if (auto C = GetLockController())
		if (C->HasNative())
			return EAGX_ConstraintControllerType::ConstraintLockController;

	if (auto C = GetRangeController())
		if (C->HasNative())
			return EAGX_ConstraintControllerType::ConstraintRangeController;

	if (auto C = GetFrictionController())
		if (C->HasNative())
			return EAGX_ConstraintControllerType::ConstraintFrictionController;

	if (auto C = GetElectricMotorController())
		if (C->HasNative())
			return EAGX_ConstraintControllerType::ConstraintElectricMotorController;

	UE_LOG(
		LogAGX, Warning,
		TEXT("Unable to get Controller Type in FSingleControllerConstraint1DOFBarrier for "
			 "Constraint '%s'."),
		*GetName());
	return EAGX_ConstraintControllerType::Invalid;
}

EAGX_ConstraintAngleControllerType FSingleControllerConstraint1DOFBarrier::GetControllerAngleType()
	const
{
	check(HasNative());

	auto ToAngleType = [](const FConstraintControllerBarrier& C)
	{
		return C.IsRotational() ? EAGX_ConstraintAngleControllerType::RotationalAngle
								: EAGX_ConstraintAngleControllerType::SeparationAngle;
	};

	if (auto C = GetTargetSpeedController())
		if (C->HasNative())
			return ToAngleType(*C);

	if (auto C = GetLockController())
		if (C->HasNative())
			return ToAngleType(*C);

	if (auto C = GetRangeController())
		if (C->HasNative())
			return ToAngleType(*C);

	if (auto C = GetFrictionController())
		if (C->HasNative())
			return ToAngleType(*C);

	if (auto C = GetElectricMotorController())
		if (C->HasNative())
			return ToAngleType(*C);

	UE_LOG(
		LogAGX, Warning,
		TEXT("Unable to get Controller Angle Type in FSingleControllerConstraint1DOFBarrier for "
			 "Constraint '%s'. Falling back on "
			 "EAGX_ConstraintAngleControllerType::RotationalAngle."),
		*GetName());
	return EAGX_ConstraintAngleControllerType::RotationalAngle;
}

void FSingleControllerConstraint1DOFBarrier::AllocateNativeImpl(
	const FRigidBodyBarrier& RigidBody1, const FVector& FramePosition1, const FQuat& FrameRotation1,
	const FRigidBodyBarrier* RigidBody2, const FVector& FramePosition2, const FQuat& FrameRotation2)
{
	UE_LOG(
		LogAGX, Error,
		TEXT("Wrong variant of AllocateNativeImpl called on "
			 "FSingleControllerConstraint1DOFBarrier. No Native object will be created."));
	AGX_CHECK(false);
}
