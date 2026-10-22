// Copyright 2026, Algoryx Simulation AB.

#include "Utilities/ROS2Utilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agx-nt-ros2/DDSBackend.h>
#include "EndAGXIncludes.h"

void AGX_ROS2Utilities::SetBackend(EAGX_ROS2Backend Backend)
{
	switch (Backend)
	{
		case EAGX_ROS2Backend::FastDDS:
			agxROS2::setBackend(agxROS2::Backend::FastDDS);
			return;
		case EAGX_ROS2Backend::CycloneDDS:
			agxROS2::setBackend(agxROS2::Backend::CycloneDDS);
			return;
	}

	UE_LOG(LogAGX, Warning, TEXT("Invalid ROS2 Backend setting. Using Fast DDS."));
	agxROS2::setBackend(agxROS2::Backend::FastDDS);
}
