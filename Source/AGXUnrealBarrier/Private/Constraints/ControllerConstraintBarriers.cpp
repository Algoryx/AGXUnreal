// Copyright 2025, Algoryx Simulation AB.

#include "Constraints/ControllerConstraintBarriers.h"

// AGX Dynamics for Unreal includes.
#include "TypeConversions.h"
#include "BarrierOnly/AGXRefs.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/Constraint.h>
#include "EndAGXIncludes.h"

FConstraintControllerBarrier::FConstraintControllerBarrier()
	: NativeRef {std::make_unique<FConstraintControllerRef>()}
{
}

FConstraintControllerBarrier::FConstraintControllerBarrier(
	std::unique_ptr<FConstraintControllerRef> Native)
	: NativeRef(std::move(Native))
{
	check(NativeRef.get());
}

FConstraintControllerBarrier::~FConstraintControllerBarrier()
{
	// Must have a non-inlined destructor because the inlined NativeRef
	// destructor must be able to see the full definition of
	// FConstraintControllerRef.
}

bool FConstraintControllerBarrier::HasNative() const
{
	return NativeRef.get() != nullptr && NativeRef->Native != nullptr;
}

FConstraintControllerRef* FConstraintControllerBarrier::GetNative()
{
	return NativeRef.get();
}

const FConstraintControllerRef* FConstraintControllerBarrier::GetNative() const
{
	return NativeRef.get();
}

void FConstraintControllerBarrier::SetEnable(bool bEnabled)
{
	check(HasNative());
	NativeRef->Native->setEnable(bEnabled);
}

bool FConstraintControllerBarrier::GetEnable() const
{
	check(HasNative());
	return NativeRef->Native->getEnable();
}

void FConstraintControllerBarrier::SetCompliance(double Compliance)
{
	check(HasNative());
	NativeRef->Native->setCompliance(Compliance);
}

double FConstraintControllerBarrier::GetCompliance() const
{
	check(HasNative());
	return NativeRef->Native->getCompliance();
}

void FConstraintControllerBarrier::SetElasticity(double Elasticity)
{
	check(HasNative());
	NativeRef->Native->setElasticity(Elasticity);
}

double FConstraintControllerBarrier::GetElasticity() const
{
	check(HasNative());
	return NativeRef->Native->getElasticity();
}

void FConstraintControllerBarrier::SetSpookDamping(double SpookDamping)
{
	check(HasNative());
	NativeRef->Native->setDamping(SpookDamping);
}

double FConstraintControllerBarrier::GetSpookDamping() const
{
	check(HasNative());
	return NativeRef->Native->getDamping();
}

void FConstraintControllerBarrier::SetForceRange(FAGX_RealInterval ForceRange)
{
	check(HasNative());
	const agx::RangeReal ForceRangeAGX = Convert(ForceRange);
	NativeRef->Native->setForceRange(ForceRangeAGX);
}

FAGX_RealInterval FConstraintControllerBarrier::GetForceRange() const
{
	check(HasNative());
	const agx::RangeReal ForceRangeAGX = NativeRef->Native->getForceRange();
	return Convert(ForceRangeAGX);
}

double FConstraintControllerBarrier::GetForce() const
{
	check(HasNative());
	return NativeRef->Native->getCurrentForce();
}

void FConstraintControllerBarrier::SetName(const FString& Name)
{
	check(HasNative());
	NativeRef->Native->setName(Convert(Name));
}

FString FConstraintControllerBarrier::GetName() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getName());
}

bool FConstraintControllerBarrier::IsRotational() const
{
	check(HasNative());
	const bool bRotational = NativeRef->Native->getData().getAngle()->is<agx::RotationalAngle>();
	return bRotational;
}

namespace
{
	template <typename AGXController>
	AGXController* GetTypedController(FConstraintControllerBarrier& Barrier)
	{
		return dynamic_cast<AGXController*>(Barrier.GetNative()->Native.get());
	}

	template <typename AGXController>
	AGXController* GetTypedController(const FConstraintControllerBarrier& Barrier)
	{
		return dynamic_cast<AGXController*>(Barrier.GetNative()->Native.get());
	}
}

//
// Electric Motor Controller starts here.
//

namespace
{
	agx::ElectricMotorController* GetController(FElectricMotorControllerBarrier& Barrier)
	{
		return GetTypedController<agx::ElectricMotorController>(Barrier);
	}

	const agx::ElectricMotorController* GetController(
		const FElectricMotorControllerBarrier& Barrier)
	{
		return GetTypedController<const agx::ElectricMotorController>(Barrier);
	}
}

FElectricMotorControllerBarrier::FElectricMotorControllerBarrier(
	std::unique_ptr<FConstraintControllerRef> Native)
	: FConstraintControllerBarrier(std::move(Native))
{
	if (Native != nullptr)
	{
		check(NativeRef->Native->is<agx::ElectricMotorController>());
	}
}

void FElectricMotorControllerBarrier::SetVoltage(double Voltage)
{
	check(HasNative());
	GetController(*this)->setVoltage(Voltage);
}

double FElectricMotorControllerBarrier::GetVoltage() const
{
	check(HasNative());
	return GetController(*this)->getVoltage();
}

void FElectricMotorControllerBarrier::SetArmatureResistance(double ArmatureResistance)
{
	check(HasNative());
	GetController(*this)->setArmatureResistance(ArmatureResistance);
}

double FElectricMotorControllerBarrier::GetArmatureResistance() const
{
	check(HasNative());
	return GetController(*this)->getArmatureResistance();
}

void FElectricMotorControllerBarrier::SetTorqueConstant(double TorqueConstant)
{
	check(HasNative());
	GetController(*this)->setTorqueConstant(TorqueConstant);
}

double FElectricMotorControllerBarrier::GetTorqueConstant() const
{
	check(HasNative());
	return GetController(*this)->getTorqueConstant();
}

//
// Friction Controller starts here.
//

namespace
{
	agx::FrictionController* GetController(FFrictionControllerBarrier& Barrier)
	{
		return GetTypedController<agx::FrictionController>(Barrier);
	}

	const agx::FrictionController* GetController(const FFrictionControllerBarrier& Barrier)
	{
		return GetTypedController<const agx::FrictionController>(Barrier);
	}
}

FFrictionControllerBarrier::FFrictionControllerBarrier(
	std::unique_ptr<FConstraintControllerRef> Native)
	: FConstraintControllerBarrier(std::move(Native))
{
	if (Native != nullptr)
	{
		check(NativeRef->Native->is<agx::FrictionController>());
	}
}

void FFrictionControllerBarrier::SetFrictionCoefficient(double FrictionCoefficient)
{
	check(HasNative());
	GetController(*this)->setFrictionCoefficient(FrictionCoefficient);
}

double FFrictionControllerBarrier::GetFrictionCoefficient() const
{
	check(HasNative());
	return GetController(*this)->getFrictionCoefficient();
}

void FFrictionControllerBarrier::SetEnableNonLinearDirectSolveUpdate(bool bEnable)
{
	check(HasNative());
	GetController(*this)->setEnableNonLinearDirectSolveUpdate(bEnable);
}

bool FFrictionControllerBarrier::GetEnableNonLinearDirectSolveUpdate() const
{
	check(HasNative());
	return GetController(*this)->getEnableNonLinearDirectSolveUpdate();
}

//
// Lock Controller starts here.
//

namespace
{
	agx::LockController* GetController(FLockControllerBarrier& Barrier)
	{
		return GetTypedController<agx::LockController>(Barrier);
	}

	const agx::LockController* GetController(const FLockControllerBarrier& Barrier)
	{
		return GetTypedController<const agx::LockController>(Barrier);
	}
}

FLockControllerBarrier::FLockControllerBarrier(std::unique_ptr<FConstraintControllerRef> Native)
	: FConstraintControllerBarrier(std::move(Native))
{
	if (Native != nullptr)
	{
		check(NativeRef->Native->is<agx::LockController>());
	}
}

void FLockControllerBarrier::SetPosition(double Position)
{
	check(HasNative());
	const bool Rotational = IsRotational();
	const agx::Real PositionAGX =
		Rotational ? ConvertAngleToAGX(Position) : ConvertDistanceToAGX(Position);
	GetController(*this)->setPosition(PositionAGX);
}

double FLockControllerBarrier::GetPosition() const
{
	check(HasNative());
	const agx::Real PositionAGX = GetController(*this)->getPosition();
	const bool Rotational = IsRotational();
	const double PositionUnreal = Rotational ? ConvertAngleToUnreal<double>(PositionAGX)
											 : ConvertDistanceToUnreal<double>(PositionAGX);
	return PositionUnreal;
}

//
// Range Controller starts here.
//

namespace
{
	agx::RangeController* GetController(FRangeControllerBarrier& Barrier)
	{
		return GetTypedController<agx::RangeController>(Barrier);
	}

	const agx::RangeController* GetController(const FRangeControllerBarrier& Barrier)
	{
		return GetTypedController<const agx::RangeController>(Barrier);
	}
}

FRangeControllerBarrier::FRangeControllerBarrier(std::unique_ptr<FConstraintControllerRef> Native)
	: FConstraintControllerBarrier(std::move(Native))
{
	if (Native != nullptr)
	{
		check(NativeRef->Native->is<agx::RangeController>());
	}
}

void FRangeControllerBarrier::SetRange(FAGX_RealInterval Range)
{
	check(HasNative());
	const bool Rotational = IsRotational();
	agx::RangeReal RangeAGX = Rotational ? ConvertAngle(Range) : ConvertDistance(Range);
	GetController(*this)->setRange(RangeAGX);
}

FAGX_RealInterval FRangeControllerBarrier::GetRange() const
{
	check(HasNative());
	agx::RangeReal RangeAGX = GetController(*this)->getRange();
	const bool Rotational = IsRotational();
	FAGX_RealInterval RangeUnreal = Rotational ? ConvertAngle(RangeAGX) : ConvertDistance(RangeAGX);
	return RangeUnreal;
}

//
// Screw Controller starts here.
//

namespace
{
	agx::ScrewController* GetController(FScrewControllerBarrier& Barrier)
	{
		return GetTypedController<agx::ScrewController>(Barrier);
	}

	const agx::ScrewController* GetController(const FScrewControllerBarrier& Barrier)
	{
		return GetTypedController<const agx::ScrewController>(Barrier);
	}
}

FScrewControllerBarrier::FScrewControllerBarrier(std::unique_ptr<FConstraintControllerRef> Native)
	: FConstraintControllerBarrier(std::move(Native))
{
	if (Native != nullptr)
	{
		check(NativeRef->Native->is<agx::ScrewController>());
	}
}

void FScrewControllerBarrier::SetLead(double Lead)
{
	check(HasNative());
	agx::Real LeadAGX = ConvertDistanceToAGX(Lead);
	GetController(*this)->setLead(LeadAGX);
}

double FScrewControllerBarrier::GetLead() const
{
	check(HasNative());
	agx::Real LeadAGX = GetController(*this)->getLead();
	return ConvertDistanceToUnreal<double>(LeadAGX);
}

//
// Target Speed  Controller starts here.
//

namespace
{
	agx::TargetSpeedController* GetController(FTargetSpeedControllerBarrier& Barrier)
	{
		return GetTypedController<agx::TargetSpeedController>(Barrier);
	}

	const agx::TargetSpeedController* GetController(const FTargetSpeedControllerBarrier& Barrier)
	{
		return GetTypedController<const agx::TargetSpeedController>(Barrier);
	}
}

FTargetSpeedControllerBarrier::FTargetSpeedControllerBarrier(
	std::unique_ptr<FConstraintControllerRef> Native)
	: FConstraintControllerBarrier(std::move(Native))
{
	if (Native != nullptr)
	{
		check(NativeRef->Native->is<agx::TargetSpeedController>());
	}
}

void FTargetSpeedControllerBarrier::SetSpeed(double Speed)
{
	check(HasNative());
	const bool Rotational = IsRotational();
	const agx::Real SpeedAGX = Rotational ? ConvertAngleToAGX(Speed) : ConvertDistanceToAGX(Speed);
	GetController(*this)->setSpeed(SpeedAGX);
}

double FTargetSpeedControllerBarrier::GetSpeed() const
{
	check(HasNative());
	const agx::Real SpeedAGX = GetController(*this)->getSpeed();
	const bool Rotational = IsRotational();
	const double SpeedUnreal = Rotational ? ConvertAngleToUnreal<double>(SpeedAGX)
										  : ConvertDistanceToUnreal<double>(SpeedAGX);
	return SpeedUnreal;
}

void FTargetSpeedControllerBarrier::SetLockedAtZeroSpeed(bool LockedAtZeroSpeed)
{
	check(HasNative());
	GetController(*this)->setLockedAtZeroSpeed(LockedAtZeroSpeed);
}

bool FTargetSpeedControllerBarrier::GetLockedAtZeroSpeed() const
{
	check(HasNative());
	return GetController(*this)->getLockedAtZeroSpeed();
}
