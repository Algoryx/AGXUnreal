// Copyright 2024, Algoryx Simulation AB.

#include "Constraints/ControllerConstraintBarriers.h"

// AGX Dynamics for Unreal includes.
#include "TypeConversions.h"
#include "BarrierOnly/AGXRefs.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx/Constraint.h>
#include "EndAGXIncludes.h"

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
	/// \todo Should the ForceRange be in N or Unreal-newtons? Unreal-newtons
	/// is kg cm s^-2 instead of kg m s^-2.
	///       ^                     ^
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
	check(HasNative());
	check(NativeRef->Native->is<agx::ElectricMotorController>());
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
	check(HasNative());
	check(NativeRef->Native->is<agx::FrictionController>());
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
	check(HasNative());
	check(NativeRef->Native->is<agx::LockController>());
}

void FLockControllerBarrier::SetPositionTranslational(double Position)
{
	check(HasNative());
	const agx::Real PositionAGX = ConvertDistanceToAGX(Position);
	GetController(*this)->setPosition(PositionAGX);
}

double FLockControllerBarrier::GetPositionTranslational() const
{
	check(HasNative());
	const agx::Real PositionAGX = GetController(*this)->getPosition();
	const double PositionUnreal = ConvertDistanceToUnreal<double>(PositionAGX);
	return PositionUnreal;
}

void FLockControllerBarrier::SetPositionRotational(double Degrees)
{
	check(HasNative());
	const agx::Real Radians = ConvertAngleToAGX(Degrees);
	GetController(*this)->setPosition(Radians);
}

double FLockControllerBarrier::GetPositionRotational() const
{
	check(HasNative());
	const agx::Real Radians = GetController(*this)->getPosition();
	const double Degrees = ConvertAngleToUnreal<double>(Radians);
	return Degrees;
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
	check(HasNative());
	check(NativeRef->Native->is<agx::RangeController>());
}

void FRangeControllerBarrier::SetRangeTranslational(FAGX_RealInterval Range)
{
	check(HasNative());
	agx::RangeReal RangeAGX = ConvertDistance(Range);
	GetController(*this)->setRange(RangeAGX);
}

FAGX_RealInterval FRangeControllerBarrier::GetRangeTranslational() const
{
	check(HasNative());
	agx::RangeReal RangeAGX = GetController(*this)->getRange();
	FAGX_RealInterval RangeUnreal = ConvertDistance(RangeAGX);
	return RangeUnreal;
}

void FRangeControllerBarrier::SetRangeRotational(FAGX_RealInterval Range)
{
	check(HasNative());
	agx::RangeReal RangeAGX = ConvertAngle(Range);
	GetController(*this)->setRange(RangeAGX);
}

FAGX_RealInterval FRangeControllerBarrier::GetRangeRotational() const
{
	check(HasNative());
	agx::RangeReal RangeAGX = GetController(*this)->getRange();
	FAGX_RealInterval RangeUnreal = ConvertAngle(RangeAGX);
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
	check(HasNative());
	check(NativeRef->Native->is<agx::ScrewController>());
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
	check(HasNative());
	check(NativeRef->Native->is<agx::TargetSpeedController>());
}

void FTargetSpeedControllerBarrier::SetSpeedTranslational(double Speed)
{
	check(HasNative());
	const agx::Real SpeedAGX = ConvertDistanceToAGX(Speed);
	GetController(*this)->setSpeed(SpeedAGX);
}

double FTargetSpeedControllerBarrier::GetSpeedTranslational() const
{
	check(HasNative());
	const agx::Real SpeedAGX = GetController(*this)->getSpeed();
	const double SpeedUnreal = ConvertDistanceToUnreal<double>(SpeedAGX);
	return SpeedUnreal;
}

void FTargetSpeedControllerBarrier::SetSpeedRotational(double Speed)
{
	check(HasNative());
	const agx::Real SpeedAGX = ConvertAngleToAGX(Speed);
	GetController(*this)->setSpeed(SpeedAGX);
}

double FTargetSpeedControllerBarrier::GetSpeedRotational() const
{
	check(HasNative());
	const agx::Real SpeedAGX = GetController(*this)->getSpeed();
	const double SpeedUnreal = ConvertAngleToUnreal<double>(SpeedAGX);
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
