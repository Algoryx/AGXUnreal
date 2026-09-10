// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/CameraCMOSSensorBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "BarrierOnly/AGXTypeConversions.h"
#include "Sensors/SensorRef.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/CameraCMOSSensor.h>
#include "EndAGXIncludes.h"

namespace CameraCMOSSensorBarrier_helpers
{
	agxSensor::CameraCMOSSensor* GetNative(FCameraCMOSSensorBarrier& Barrier)
	{
		check(Barrier.HasNative());
		agxSensor::CameraCMOSSensor* Sensor =
			Barrier.GetNative()->Native->asSafe<agxSensor::CameraCMOSSensor>();
		AGX_CHECK(Sensor != nullptr);
		return Sensor;
	}

	const agxSensor::CameraCMOSSensor* GetNative(const FCameraCMOSSensorBarrier& Barrier)
	{
		check(Barrier.HasNative());
		const agxSensor::CameraCMOSSensor* Sensor =
			Barrier.GetNative()->Native->asSafe<agxSensor::CameraCMOSSensor>();
		AGX_CHECK(Sensor != nullptr);
		return Sensor;
	}
}

FCameraCMOSSensorBarrier::FCameraCMOSSensorBarrier(
	std::shared_ptr<FCameraPhotodetectorRef> Native)
	: FCameraPhotodetectorBarrier(std::move(Native))
{
}

void FCameraCMOSSensorBarrier::AllocateNative()
{
	check(!HasNative());
	NativeRef->Native = new agxSensor::CameraCMOSSensor();
}

void FCameraCMOSSensorBarrier::SetSize(const FVector2D& InSize)
{
	CameraCMOSSensorBarrier_helpers::GetNative(*this)->setSize(ConvertDistance(InSize));
}

FVector2D FCameraCMOSSensorBarrier::GetSize() const
{
	return ConvertDistance(CameraCMOSSensorBarrier_helpers::GetNative(*this)->getSize());
}

void FCameraCMOSSensorBarrier::SetISO(double InISO)
{
	CameraCMOSSensorBarrier_helpers::GetNative(*this)->setISO(InISO);
}

double FCameraCMOSSensorBarrier::GetISO() const
{
	return CameraCMOSSensorBarrier_helpers::GetNative(*this)->getISO();
}

void FCameraCMOSSensorBarrier::SetShutterSpeed(double InShutterSpeed)
{
	CameraCMOSSensorBarrier_helpers::GetNative(*this)->setShutterSpeed(InShutterSpeed);
}

double FCameraCMOSSensorBarrier::GetShutterSpeed() const
{
	return CameraCMOSSensorBarrier_helpers::GetNative(*this)->getShutterSpeed();
}

void FCameraCMOSSensorBarrier::SetAutoExposure(double InDynamicRange)
{
	CameraCMOSSensorBarrier_helpers::GetNative(*this)->setAutoExposure(InDynamicRange);
}

bool FCameraCMOSSensorBarrier::GetUseAutoExposure() const
{
	return CameraCMOSSensorBarrier_helpers::GetNative(*this)->hasAutoExposureEnabled();
}

void FCameraCMOSSensorBarrier::SetManualExposureCompensation(double InExposureCompensation)
{
	CameraCMOSSensorBarrier_helpers::GetNative(*this)->setManualExposureCompensation(
		InExposureCompensation);
}

double FCameraCMOSSensorBarrier::GetDynamicRange() const
{
	const auto DynamicRange =
		CameraCMOSSensorBarrier_helpers::GetNative(*this)->getDynamicRange();
	return DynamicRange.has_value() ? *DynamicRange : 0.0;
}

double FCameraCMOSSensorBarrier::GetExposureCompensation() const
{
	const auto ExposureCompensation =
		CameraCMOSSensorBarrier_helpers::GetNative(*this)->getExposureCompensation();
	return ExposureCompensation.has_value() ? *ExposureCompensation : 0.0;
}

bool FCameraCMOSSensorBarrier::IsCMOSSensor(const FCameraPhotodetectorBarrier& Photodetector)
{
	if (!Photodetector.HasNative())
		return false;

	return Photodetector.GetNative()->Native->is<agxSensor::CameraCMOSSensor>();
}
