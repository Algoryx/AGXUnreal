// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_RealInterval.h"

// Unreal Engine includes.
#include "Math/Interval.h"

// Standard library includes.
#include <memory>

struct FConstraintControllerRef;

class AGXUNREALBARRIER_API FConstraintControllerBarrier
{
public:
	virtual ~FConstraintControllerBarrier();

	bool HasNative() const;
	FConstraintControllerRef* GetNative();
	const FConstraintControllerRef* GetNative() const;

	void SetEnable(bool bEnabled);
	bool GetEnable() const;

	void SetCompliance(double Compliance);
	double GetCompliance() const;

	void SetElasticity(double Elasticity);
	double GetElasticity() const;

	void SetSpookDamping(double SpookDamping);
	double GetSpookDamping() const;

	void SetForceRange(FAGX_RealInterval ForceRange);
	FAGX_RealInterval GetForceRange() const;

	double GetForce() const;

	void SetName(const FString& Name);
	FString GetName() const;

	bool IsRotational() const;

protected:
	FConstraintControllerBarrier();
	FConstraintControllerBarrier(std::unique_ptr<FConstraintControllerRef> Native);

private:
	FConstraintControllerBarrier(const FConstraintControllerBarrier&) = delete;
	void operator=(const FConstraintControllerBarrier&) = delete;

protected:
	std::unique_ptr<FConstraintControllerRef> NativeRef;
};

class AGXUNREALBARRIER_API FElectricMotorControllerBarrier : public FConstraintControllerBarrier
{
public:
	FElectricMotorControllerBarrier() = default;
	FElectricMotorControllerBarrier(std::unique_ptr<FConstraintControllerRef> Native);

	void SetVoltage(double Voltage);
	double GetVoltage() const;

	void SetArmatureResistance(double ArmatureResistance);
	double GetArmatureResistance() const;

	void SetTorqueConstant(double TorqueConstant);
	double GetTorqueConstant() const;
};

class AGXUNREALBARRIER_API FFrictionControllerBarrier : public FConstraintControllerBarrier
{
public:
	FFrictionControllerBarrier() = default;
	FFrictionControllerBarrier(std::unique_ptr<FConstraintControllerRef> Native);

	void SetFrictionCoefficient(double FrictionCoefficient);
	double GetFrictionCoefficient() const;

	void SetEnableNonLinearDirectSolveUpdate(bool bEnable);
	bool GetEnableNonLinearDirectSolveUpdate() const;
};

class AGXUNREALBARRIER_API FLockControllerBarrier : public FConstraintControllerBarrier
{
public:
	FLockControllerBarrier() = default;
	FLockControllerBarrier(std::unique_ptr<FConstraintControllerRef> Native);

	void SetPosition(double Position);
	double GetPosition() const;
};

class AGXUNREALBARRIER_API FRangeControllerBarrier : public FConstraintControllerBarrier
{
public:
	FRangeControllerBarrier() = default;
	FRangeControllerBarrier(std::unique_ptr<FConstraintControllerRef> Native);

	void SetRange(FAGX_RealInterval Range);
	FAGX_RealInterval GetRange() const;
};

class AGXUNREALBARRIER_API FScrewControllerBarrier : public FConstraintControllerBarrier
{
public:
	FScrewControllerBarrier() = default;
	FScrewControllerBarrier(std::unique_ptr<FConstraintControllerRef> Native);

	void SetLead(double Lead);
	double GetLead() const;
};

class AGXUNREALBARRIER_API FTargetSpeedControllerBarrier : public FConstraintControllerBarrier
{
public:
	FTargetSpeedControllerBarrier() = default;
	FTargetSpeedControllerBarrier(std::unique_ptr<FConstraintControllerRef> Native);

	void SetSpeed(double Speed);
	double GetSpeed() const;

	void SetLockedAtZeroSpeed(bool LockedAtZeroSpeed);
	bool GetLockedAtZeroSpeed() const;
};
