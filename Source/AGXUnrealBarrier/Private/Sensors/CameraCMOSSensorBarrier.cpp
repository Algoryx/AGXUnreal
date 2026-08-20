// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/CameraCMOSSensorBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/SensorRef.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/CameraCMOSSensor.h>
#include "EndAGXIncludes.h"

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

bool FCameraCMOSSensorBarrier::IsCMOSSensor(const FCameraPhotodetectorBarrier& Photodetector)
{
	if (!Photodetector.HasNative())
		return false;

	return Photodetector.GetNative()->Native->is<agxSensor::CameraCMOSSensor>();
}
