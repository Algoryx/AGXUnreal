// Copyright 2026, Algoryx Simulation AB.

#include "Sensors/CameraOutputColorBarrier.h"

// AGX Dynamics for Unreal includes.
#include "Sensors/SensorRef.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxSensor/CameraColorOutput.h>
#include "EndAGXIncludes.h"

FCameraOutputColorBarrier::FCameraOutputColorBarrier(std::shared_ptr<FCameraOutputRef> Native)
	: FCameraOutputBarrier(std::move(Native))
{
}

void FCameraOutputColorBarrier::AllocateNative()
{
	check(!HasNative());
	NativeRef->Native = new agxSensor::CameraColorOutput();
}

bool FCameraOutputColorBarrier::IsColorOutput(const FCameraOutputBarrier& Output)
{
	if (!Output.HasNative())
		return false;

	return Output.GetNative()->Native->is<agxSensor::CameraColorOutput>();
}
