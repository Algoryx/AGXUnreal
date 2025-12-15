// Copyright 2025, Algoryx Simulation AB.

#include "Constraints/Constraint2DOFBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/AGXTypeConversions.h"
#include "Constraints/AGX_Constraint2DOFFreeDOF.h"
#include "Constraints/ControllerConstraintBarriers.h"
#include "RigidBodyBarrier.h"
#include "Utilities/AGX_BarrierConstraintUtilities.h"

FConstraint2DOFBarrier::FConstraint2DOFBarrier()
	: FConstraintBarrier()
{
}

FConstraint2DOFBarrier::FConstraint2DOFBarrier(std::unique_ptr<FConstraintRef> Native)
	: FConstraintBarrier(std::move(Native))
{
}

FConstraint2DOFBarrier::~FConstraint2DOFBarrier()
{
}

// A collection of template specializations to fetch the AGX Dynamics controller constraint
// from an AGX Dynamics Constraint2DOF. Each template specialization knows if getMotor1D,
// getRange1D, or something else should be called for the particular Barrier type that the template
// is specialized for.
namespace
{
	agx::Constraint2DOF* Get2DOF(std::unique_ptr<FConstraintRef>& NativeRef)
	{
		return dynamic_cast<agx::Constraint2DOF*>(NativeRef->Native.get());
	}

	agx::Constraint2DOF* Get2DOF(const std::unique_ptr<FConstraintRef>& NativeRef)
	{
		return dynamic_cast<agx::Constraint2DOF*>(NativeRef->Native.get());
	}

	agx::Constraint2DOF* Get2DOF(FConstraintRef* NativeRef)
	{
		return dynamic_cast<agx::Constraint2DOF*>(NativeRef->Native.get());
	}

	template <typename FControllerBarrier, typename FControllerGetter>
	TUniquePtr<FControllerBarrier> CreateControllerBarrier(
		const std::unique_ptr<FConstraintRef>& NativeRef, FControllerGetter ControllerGetter)
	{
		check(NativeRef != nullptr && NativeRef->Native != nullptr);
		auto* Controller = ControllerGetter(Get2DOF(NativeRef));
		return TUniquePtr<FControllerBarrier>(
			new FControllerBarrier(std::make_unique<FConstraintControllerRef>(Controller)));
	}
}

double FConstraint2DOFBarrier::GetAngle(EAGX_Constraint2DOFFreeDOF Dof) const
{
	check(NativeRef != nullptr && NativeRef->Native != nullptr);
	agx::Constraint2DOF* Constraint = Get2DOF(NativeRef);
	const agx::Motor1D* MotorAGX = Constraint->getMotor1D(Convert(Dof));
	const agx::Angle::Type DofType = FAGX_BarrierConstraintUtilities::GetDofType(MotorAGX);
	const agx::Real NativeAngle = Constraint->getAngle(Convert(Dof));
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

double FConstraint2DOFBarrier::GetSpeed(EAGX_Constraint2DOFFreeDOF Dof) const
{
	check(HasNative());

	agx::Constraint2DOF* Constraint = Get2DOF(NativeRef);
	const agx::Real SpeedAGX = Constraint->getCurrentSpeed(Convert(Dof));
	const agx::Motor1D* MotorAGX = Constraint->getMotor1D(Convert(Dof));
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

TUniquePtr<FElectricMotorControllerBarrier> FConstraint2DOFBarrier::GetElectricMotorController(
	EAGX_Constraint2DOFFreeDOF Dof)
{
	return CreateControllerBarrier<FElectricMotorControllerBarrier>(
		NativeRef, [Dof](agx::Constraint2DOF* Constraint)
		{ return Constraint->getElectricMotorController(Convert(Dof)); });
}

TUniquePtr<FFrictionControllerBarrier> FConstraint2DOFBarrier::GetFrictionController(
	EAGX_Constraint2DOFFreeDOF Dof)
{
	/// \todo Why not using GetOrCreateFrictionController here?
	return CreateControllerBarrier<FFrictionControllerBarrier>(
		NativeRef, [Dof](agx::Constraint2DOF* Constraint)
		{ return Constraint->getFrictionController(Convert(Dof)); });
}

TUniquePtr<FLockControllerBarrier> FConstraint2DOFBarrier::GetLockController(
	EAGX_Constraint2DOFFreeDOF Dof)
{
	return CreateControllerBarrier<FLockControllerBarrier>(
		NativeRef,
		[Dof](agx::Constraint2DOF* Constraint) { return Constraint->getLock1D(Convert(Dof)); });
}

TUniquePtr<FRangeControllerBarrier> FConstraint2DOFBarrier::GetRangeController(
	EAGX_Constraint2DOFFreeDOF Dof)
{
	return CreateControllerBarrier<FRangeControllerBarrier>(
		NativeRef,
		[Dof](agx::Constraint2DOF* Constraint) { return Constraint->getRange1D(Convert(Dof)); });
}

TUniquePtr<FTargetSpeedControllerBarrier> FConstraint2DOFBarrier::GetTargetSpeedController(
	EAGX_Constraint2DOFFreeDOF Dof)
{
	return CreateControllerBarrier<FTargetSpeedControllerBarrier>(
		NativeRef,
		[Dof](agx::Constraint2DOF* Constraint) { return Constraint->getMotor1D(Convert(Dof)); });
}

TUniquePtr<FScrewControllerBarrier> FConstraint2DOFBarrier::GetScrewController()
{
	return CreateControllerBarrier<FScrewControllerBarrier>(
		NativeRef, [](agx::Constraint2DOF* Constraint) { return Constraint->getScrew1D(); });
}

TUniquePtr<const FElectricMotorControllerBarrier>
FConstraint2DOFBarrier::GetElectricMotorController(EAGX_Constraint2DOFFreeDOF Dof) const
{
	return CreateControllerBarrier<const FElectricMotorControllerBarrier>(
		NativeRef, [Dof](agx::Constraint2DOF* Constraint)
		{ return Constraint->getElectricMotorController(Convert(Dof)); });
}

namespace
{
	agx::FrictionController* GetOrCreateFrictionController(
		agx::Constraint2DOF* Constraint, agx::Constraint2DOF::DOF Dof)
	{
		agx::FrictionController* Friction = Constraint->getFrictionController(Dof);
		if (Friction == nullptr)
		{
			agx::AttachmentPair* Attachments = Constraint->getAttachmentPair();
			agx::Angle* SepAngle = Attachments->getAngle(0);
			agx::Angle* RotAngle = Attachments->getAngle(1);
			agx::ConstraintAngleBasedData SepData(Attachments, SepAngle);
			agx::ConstraintAngleBasedData RotData(Attachments, RotAngle);
			agx::FrictionController* SepFriction = new agx::FrictionController(SepData);
			agx::FrictionController* RotFriction = new agx::FrictionController(RotData);
			Constraint->addSecondaryConstraint("FT", SepFriction);
			Constraint->addSecondaryConstraint("FR", RotFriction);
			Friction = Dof == agx::Constraint2DOF::FIRST ? SepFriction : RotFriction;
			check(Friction == Constraint->getFrictionController(Dof));
		}
		return Friction;
	}
}

TUniquePtr<const FFrictionControllerBarrier> FConstraint2DOFBarrier::GetFrictionController(
	EAGX_Constraint2DOFFreeDOF Dof) const
{
	return CreateControllerBarrier<const FFrictionControllerBarrier>(
		NativeRef, [Dof](agx::Constraint2DOF* Constraint)
		{ return GetOrCreateFrictionController(Constraint, Convert(Dof)); });
}

TUniquePtr<const FLockControllerBarrier> FConstraint2DOFBarrier::GetLockController(
	EAGX_Constraint2DOFFreeDOF Dof) const
{
	return CreateControllerBarrier<const FLockControllerBarrier>(
		NativeRef,
		[Dof](agx::Constraint2DOF* Constraint) { return Constraint->getLock1D(Convert(Dof)); });
}

TUniquePtr<const FRangeControllerBarrier> FConstraint2DOFBarrier::GetRangeController(
	EAGX_Constraint2DOFFreeDOF Dof) const
{
	return CreateControllerBarrier<const FRangeControllerBarrier>(
		NativeRef,
		[Dof](agx::Constraint2DOF* Constraint) { return Constraint->getRange1D(Convert(Dof)); });
}

TUniquePtr<const FTargetSpeedControllerBarrier> FConstraint2DOFBarrier::GetTargetSpeedController(
	EAGX_Constraint2DOFFreeDOF Dof) const
{
	return CreateControllerBarrier<const FTargetSpeedControllerBarrier>(
		NativeRef,
		[Dof](agx::Constraint2DOF* Constraint) { return Constraint->getMotor1D(Convert(Dof)); });
}

TUniquePtr<const FScrewControllerBarrier> FConstraint2DOFBarrier::GetScrewController() const
{
	return CreateControllerBarrier<const FScrewControllerBarrier>(
		NativeRef, [](agx::Constraint2DOF* Constraint) { return Constraint->getScrew1D(); });
}
