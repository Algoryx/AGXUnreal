// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/CameraLensSingleElementBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/SensorRef.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/CameraLensSingleElement.h>
#include "EndAGXIncludes.h"

FCameraLensSingleElementBarrier::FCameraLensSingleElementBarrier(
	std::shared_ptr<FCameraLensRef> Native)
	: FCameraLensBarrier(std::move(Native))
{
}

void FCameraLensSingleElementBarrier::AllocateNative()
{
	check(!HasNative());
	NativeRef->Native = new agxSensor::CameraLensSingleElement();
}

bool FCameraLensSingleElementBarrier::IsSingleElement(const FCameraLensBarrier& Lens)
{
	if (!Lens.HasNative())
		return false;

	return Lens.GetNative()->Native->is<agxSensor::CameraLensSingleElement>();
}
