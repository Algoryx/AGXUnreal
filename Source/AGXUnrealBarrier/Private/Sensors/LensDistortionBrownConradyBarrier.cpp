// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/LensDistortionBrownConradyBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "Sensors/SensorRef.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/LensDistortionBrownConrady.h>
#include "EndAGXIncludes.h"

namespace LensDistortionBrownConradyBarrier_helpers
{
	agxSensor::LensDistortionBrownConrady* GetNative(
		FLensDistortionBrownConradyBarrier& Barrier)
	{
		check(Barrier.HasNative());
		agxSensor::LensDistortionBrownConrady* Distortion =
			Barrier.GetNative()->Native->asSafe<agxSensor::LensDistortionBrownConrady>();
		AGX_CHECK(Distortion != nullptr);
		return Distortion;
	}

	const agxSensor::LensDistortionBrownConrady* GetNative(
		const FLensDistortionBrownConradyBarrier& Barrier)
	{
		check(Barrier.HasNative());
		const agxSensor::LensDistortionBrownConrady* Distortion =
			Barrier.GetNative()->Native->asSafe<agxSensor::LensDistortionBrownConrady>();
		AGX_CHECK(Distortion != nullptr);
		return Distortion;
	}
}

FLensDistortionBrownConradyBarrier::FLensDistortionBrownConradyBarrier(
	std::shared_ptr<FLensDistortionRef> Native)
	: FLensDistortionBarrier(std::move(Native))
{
}

void FLensDistortionBrownConradyBarrier::AllocateNative()
{
	check(!HasNative());
	NativeRef->Native = new agxSensor::LensDistortionBrownConrady(0.0, 0.0, 0.0, 0.0, 0.0);
}

void FLensDistortionBrownConradyBarrier::SetK1(double InK1)
{
	LensDistortionBrownConradyBarrier_helpers::GetNative(*this)->setK1(InK1);
}

double FLensDistortionBrownConradyBarrier::GetK1() const
{
	return LensDistortionBrownConradyBarrier_helpers::GetNative(*this)->getK1();
}

void FLensDistortionBrownConradyBarrier::SetK2(double InK2)
{
	LensDistortionBrownConradyBarrier_helpers::GetNative(*this)->setK2(InK2);
}

double FLensDistortionBrownConradyBarrier::GetK2() const
{
	return LensDistortionBrownConradyBarrier_helpers::GetNative(*this)->getK2();
}

void FLensDistortionBrownConradyBarrier::SetK3(double InK3)
{
	LensDistortionBrownConradyBarrier_helpers::GetNative(*this)->setK3(InK3);
}

double FLensDistortionBrownConradyBarrier::GetK3() const
{
	return LensDistortionBrownConradyBarrier_helpers::GetNative(*this)->getK3();
}

void FLensDistortionBrownConradyBarrier::SetP1(double InP1)
{
	LensDistortionBrownConradyBarrier_helpers::GetNative(*this)->setP1(InP1);
}

double FLensDistortionBrownConradyBarrier::GetP1() const
{
	return LensDistortionBrownConradyBarrier_helpers::GetNative(*this)->getP1();
}

void FLensDistortionBrownConradyBarrier::SetP2(double InP2)
{
	LensDistortionBrownConradyBarrier_helpers::GetNative(*this)->setP2(InP2);
}

double FLensDistortionBrownConradyBarrier::GetP2() const
{
	return LensDistortionBrownConradyBarrier_helpers::GetNative(*this)->getP2();
}

bool FLensDistortionBrownConradyBarrier::IsBrownConrady(
	const FLensDistortionBarrier& Distortion)
{
	if (!Distortion.HasNative())
		return false;

	return Distortion.GetNative()->Native->is<agxSensor::LensDistortionBrownConrady>();
}
