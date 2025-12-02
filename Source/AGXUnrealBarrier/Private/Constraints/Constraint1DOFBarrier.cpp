// Copyright 2025, Algoryx Simulation AB.

#include "Constraints/Constraint1DOFBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"
#include "Constraints/ControllerConstraintBarriers.h"
#include "RigidBodyBarrier.h"
#include "TypeConversions.h"
#include "Utilities/AGX_BarrierConstraintUtilities.h"

// Standard library includes.
#include <memory>

FConstraint1DOFBarrier::FConstraint1DOFBarrier()
{
}

FConstraint1DOFBarrier::FConstraint1DOFBarrier(std::unique_ptr<FConstraintRef> Native)
	: FConstraintBarrier(std::move(Native))
{
}

FConstraint1DOFBarrier::~FConstraint1DOFBarrier()
{
}

namespace
{
	agx::Constraint1DOF* Get1DOF(std::unique_ptr<FConstraintRef>& NativeRef)
	{
		return dynamic_cast<agx::Constraint1DOF*>(NativeRef->Native.get());
	}

	agx::Constraint1DOF* Get1DOF(FConstraintRef& NativeRef)
	{
		return dynamic_cast<agx::Constraint1DOF*>(NativeRef.Native.get());
	}

	agx::Constraint1DOF* Get1DOF(const std::unique_ptr<FConstraintRef>& NativeRef)
	{
		return dynamic_cast<agx::Constraint1DOF*>(NativeRef->Native.get());
	}

	agx::Constraint1DOF* Get1DOF(const FConstraintRef& NativeRef)
	{
		return dynamic_cast<agx::Constraint1DOF*>(NativeRef.Native.get());
	}

	template <typename Barrier>
	TUniquePtr<Barrier> CreateControllerBarrier(agx::BasicControllerConstraint* Controller)
	{
		return TUniquePtr<Barrier>(
			new Barrier(std::make_unique<FConstraintControllerRef>(Controller)));
	}

	// Let's hope -1 is never used for a valid angle type.
	// TODO: Find a better way than forcing a made-up enum value into an agx::Angle::Type.
	/* constexpr */ const agx::Angle::Type InvalidAngleType = static_cast<agx::Angle::Type>(-1);

	agx::Angle::Type GetDofType(const FConstraint1DOFBarrier& Constraint)
	{
		const agx::Constraint1DOF* ConstraintAGX = Get1DOF(*Constraint.GetNative());
		if (ConstraintAGX == nullptr)
		{
			return InvalidAngleType;
		}
		const agx::Motor1D* Motor = ConstraintAGX->getMotor1D();
		if (Motor == nullptr)
		{
			return InvalidAngleType;
		}
		const agx::Angle* Angle = Motor->getData().getAngle();
		if (Angle == nullptr)
		{
			return InvalidAngleType;
		}
		return Angle->getType();
	}
}

double FConstraint1DOFBarrier::GetAngle() const
{
	check(HasNative());
	const agx::Constraint1DOF* Constraint = Get1DOF(NativeRef);
	const agx::Real NativeAngle = Constraint->getAngle();
	const agx::Motor1D* MotorAGX = Constraint->getMotor1D();
	const agx::Angle::Type DofType = FAGX_BarrierConstraintUtilities::GetDofType(MotorAGX);
	switch (DofType)
	{
		case agx::Angle::ROTATIONAL:
			return ConvertAngleToUnreal<double>(NativeAngle);
		case agx::Angle::TRANSLATIONAL:
			return ConvertDistanceToUnreal<double>(NativeAngle);
		default:
			// Don't know the type, so pass the value unchanged to the caller.
			return NativeAngle;
	}
}

double FConstraint1DOFBarrier::GetSpeed() const
{
	check(HasNative());

	const agx::Constraint1DOF* Constraint = Get1DOF(NativeRef);
	const agx::Real SpeedAGX = Constraint->getCurrentSpeed();
	const agx::Motor1D* MotorAGX = Constraint->getMotor1D();
	const agx::Angle::Type DofType = FAGX_BarrierConstraintUtilities::GetDofType(MotorAGX);
	switch (DofType)
	{
		case agx::Angle::ROTATIONAL:
			return ConvertAngleToUnreal<double>(SpeedAGX);
		case agx::Angle::TRANSLATIONAL:
			return ConvertDistanceToUnreal<double>(SpeedAGX);
		default:
			// Don't know the type, so pass the value unchanged to the caller.
			return SpeedAGX;
	}
}

TUniquePtr<FElectricMotorControllerBarrier> FConstraint1DOFBarrier::GetElectricMotorController()
{
	check(HasNative());
	return CreateControllerBarrier<FElectricMotorControllerBarrier>(
		Get1DOF(NativeRef)->getElectricMotorController());
}

TUniquePtr<FFrictionControllerBarrier> FConstraint1DOFBarrier::GetFrictionController()
{
	check(HasNative());
	return CreateControllerBarrier<FFrictionControllerBarrier>(
		Get1DOF(NativeRef)->getFrictionController());
}

TUniquePtr<FLockControllerBarrier> FConstraint1DOFBarrier::GetLockController()
{
	check(HasNative());
	return CreateControllerBarrier<FLockControllerBarrier>(Get1DOF(NativeRef)->getLock1D());
}

TUniquePtr<FRangeControllerBarrier> FConstraint1DOFBarrier::GetRangeController()
{
	check(HasNative());
	return CreateControllerBarrier<FRangeControllerBarrier>(Get1DOF(NativeRef)->getRange1D());
}

TUniquePtr<FTargetSpeedControllerBarrier> FConstraint1DOFBarrier::GetTargetSpeedController()
{
	check(HasNative());
	return CreateControllerBarrier<FTargetSpeedControllerBarrier>(Get1DOF(NativeRef)->getMotor1D());
}

TUniquePtr<const FElectricMotorControllerBarrier>
FConstraint1DOFBarrier::GetElectricMotorController() const
{
	check(HasNative());
	return CreateControllerBarrier<const FElectricMotorControllerBarrier>(
		Get1DOF(NativeRef)->getElectricMotorController());
}

namespace
{
	agx::FrictionController* GetOrCreateFrictionController(agx::Constraint1DOF* Constraint)
	{
		agx::FrictionController* Friction = Constraint->getFrictionController();
		if (Friction == nullptr)
		{
			agx::AttachmentPair* Attachments = Constraint->getAttachmentPair();
			agx::Angle* Angle = Attachments->getAngle(0);
			agx::ConstraintAngleBasedData AngleData(Attachments, Angle);
			Friction = new agx::FrictionController(AngleData);
			Constraint->addSecondaryConstraint("FT", Friction);
			check(Friction == Constraint->getFrictionController());
		}
		return Friction;
	}
}

TUniquePtr<const FFrictionControllerBarrier> FConstraint1DOFBarrier::GetFrictionController() const
{
	check(HasNative());
	agx::FrictionController* Friction = GetOrCreateFrictionController(Get1DOF(NativeRef));
	return CreateControllerBarrier<const FFrictionControllerBarrier>(Friction);
}

TUniquePtr<const FLockControllerBarrier> FConstraint1DOFBarrier::GetLockController() const
{
	check(HasNative());
	return CreateControllerBarrier<const FLockControllerBarrier>(Get1DOF(NativeRef)->getLock1D());
}

TUniquePtr<const FRangeControllerBarrier> FConstraint1DOFBarrier::GetRangeController() const
{
	check(HasNative());
	return CreateControllerBarrier<const FRangeControllerBarrier>(Get1DOF(NativeRef)->getRange1D());
}

TUniquePtr<const FTargetSpeedControllerBarrier> FConstraint1DOFBarrier::GetTargetSpeedController()
	const
{
	check(HasNative());
	return CreateControllerBarrier<const FTargetSpeedControllerBarrier>(
		Get1DOF(NativeRef)->getMotor1D());
}
