// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "BeginAGXIncludes.h"
#include <agxVehicle/TrackWheel.h>
#include "EndAGXIncludes.h"

struct FTrackWheelRef
{
	agxVehicle::TrackWheelRef Native;

	FTrackWheelRef() = default;
	FTrackWheelRef(agxVehicle::TrackWheel* InNative)
		: Native(InNative)
	{
	}
};
