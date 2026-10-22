// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "ROS2/AGX_ROS2Enums.h"

namespace AGX_ROS2Utilities
{
	/**
	 * Set the process-wide DDS backend used by new ROS2 publishers and subscribers.
	 * Existing publishers and subscribers keep their backend. To use the same backend for all ROS2
	 * objects, it is recommended to call this before creating any publishers or subscribers.
	 */
	AGXUNREALBARRIER_API void SetBackend(EAGX_ROS2Backend Backend);
}
