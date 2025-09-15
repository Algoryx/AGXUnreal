// Copyright 2025, Algoryx Simulation AB.

#include "Constraints/Controllers/AGX_ElectricMotorController.h"

// AGX Dynamics for Unreal includes.
#include "Constraints/AGX_ConstraintConstants.h"
#include "Constraints/ControllerConstraintBarriers.h"

void FAGX_ConstraintElectricMotorController::InitializeBarrier(
	TUniquePtr<FElectricMotorControllerBarrier> Barrier)
{
	check(!HasNative());
	NativeBarrier = std::move(Barrier);
}

namespace
{
	FElectricMotorControllerBarrier* GetElectricMotorBarrier(
		FAGX_ConstraintElectricMotorController& Controller)
	{
		// Is there a way to guarantee that this cast is safe? We're in the
		// Unreal Engine potion of the plugin so cannot use dynamic_cast, but
		// FElectricMotorControllerBarrier doesn't inherit from UObject so we
		// cannot use Unreal Engine's Cast<> function.
		//
		// We "know" that a FAGX_ConstraintElectricMotorController will always
		// hold a FElectricMotorControllerBarrier, but there doesn't seem to be
		// a way to verify it here.
		//
		// The corresponding functions for the other ControllerBarriers reference
		// this comment. Remove those if this comment is removed.
		return static_cast<FElectricMotorControllerBarrier*>(Controller.GetNative());
	}

	const FElectricMotorControllerBarrier* GetElectricMotorBarrier(
		const FAGX_ConstraintElectricMotorController& Controller)
	{
		return static_cast<const FElectricMotorControllerBarrier*>(Controller.GetNative());
	}
}

void FAGX_ConstraintElectricMotorController::SetVoltage(double InVoltage)
{
	if (HasNative())
	{
		GetElectricMotorBarrier(*this)->SetVoltage(InVoltage);
	}
	Voltage = InVoltage;
}

double FAGX_ConstraintElectricMotorController::GetVoltage() const
{
	if (HasNative())
	{
		return GetElectricMotorBarrier(*this)->GetVoltage();
	}
	else
	{
		return Voltage;
	}
}

void FAGX_ConstraintElectricMotorController::SetArmatureResistance(double InArmatureResistance)
{
	if (HasNative())
	{
		GetElectricMotorBarrier(*this)->SetArmatureResistance(InArmatureResistance);
	}
	ArmatureResistance = InArmatureResistance;
}

void FAGX_ConstraintElectricMotorController::SetArmatureRestistance(double InArmatureResistance)
{
	SetArmatureResistance(InArmatureResistance);
}

double FAGX_ConstraintElectricMotorController::GetArmatureResistance() const
{
	if (HasNative())
	{
		return GetElectricMotorBarrier(*this)->GetArmatureResistance();
	}
	else
	{
		return ArmatureResistance;
	}
}

void FAGX_ConstraintElectricMotorController::SetTorqueConstant(double InTorqueConstant)
{
	if (HasNative())
	{
		GetElectricMotorBarrier(*this)->SetTorqueConstant(InTorqueConstant);
	}
	TorqueConstant = InTorqueConstant;
}

double FAGX_ConstraintElectricMotorController::GetTorqueConstant() const
{
	if (HasNative())
	{
		return GetElectricMotorBarrier(*this)->GetTorqueConstant();
	}
	else
	{
		return TorqueConstant;
	}
}

void FAGX_ConstraintElectricMotorController::UpdateNativePropertiesImpl()
{
	FElectricMotorControllerBarrier* Barrier = GetElectricMotorBarrier(*this);
	check(Barrier);
	Barrier->SetVoltage(Voltage);
	Barrier->SetArmatureResistance(ArmatureResistance);
	Barrier->SetTorqueConstant(TorqueConstant);
}

void FAGX_ConstraintElectricMotorController::CopyFrom(
	const FElectricMotorControllerBarrier& Source)
{
	Super::CopyFrom(Source);

	Voltage = Source.GetVoltage();
	ArmatureResistance = Source.GetArmatureResistance();
	TorqueConstant = Source.GetTorqueConstant();
}
