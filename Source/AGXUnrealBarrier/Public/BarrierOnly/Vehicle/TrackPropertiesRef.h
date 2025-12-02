// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "BeginAGXIncludes.h"
#include <agxVehicle/TrackProperties.h>
#include "EndAGXIncludes.h"

struct FTrackPropertiesRef
{
	using NativeType = agxVehicle::TrackProperties;
	agxVehicle::TrackPropertiesRef Native;

	FTrackPropertiesRef() = default;
	FTrackPropertiesRef(agxVehicle::TrackProperties* InNative)
		: Native(InNative)
	{
	}
};
