// Copyright 2026, Algoryx Simulation AB.

#pragma once

#include "BeginAGXIncludes.h"
#include <agxVehicle/Steering.h>
#include "EndAGXIncludes.h"

struct FSteeringRef
{
	agxVehicle::SteeringRef Native;

	FSteeringRef() = default;
	FSteeringRef(agxVehicle::Steering* InNative)
		: Native(InNative)
	{
	}
};
