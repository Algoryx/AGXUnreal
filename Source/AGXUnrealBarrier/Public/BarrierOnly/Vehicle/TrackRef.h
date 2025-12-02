// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "BeginAGXIncludes.h"
#include <agxVehicle/Track.h>
#include "EndAGXIncludes.h"

struct FTrackRef
{
	using NativeType = agxVehicle::Track;
	agxVehicle::TrackRef Native;

	FTrackRef() = default;
	FTrackRef(agxVehicle::Track* InNative)
		: Native(InNative)
	{
	}
};
