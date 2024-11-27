// Copyright 2024, Algoryx Simulation AB.

#include "Utilities/AGX_SensorUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Sensors/AGX_CustomRayPatternParameters.h"
#include "Sensors/AGX_GenericHorizontalSweepParameters.h"
#include "Sensors/AGX_OusterOS0Parameters.h"
#include "Sensors/AGX_OusterOS1Parameters.h"
#include "Sensors/AGX_OusterOS2Parameters.h"


UClass* FAGX_SensorUtilities::GetParameterTypeFrom(
	EAGX_LidarModel Model)
{
	switch (Model)
	{
		case EAGX_LidarModel::CustomRayPattern:
			return UAGX_CustomRayPatternParameters::StaticClass();
		case EAGX_LidarModel::GenericHorizontalSweep:
			return UAGX_GenericHorizontalSweepParameters::StaticClass();
		case EAGX_LidarModel::OusterOS0:
			return UAGX_OusterOS0Parameters::StaticClass();
		case EAGX_LidarModel::OusterOS1:
			return UAGX_OusterOS1Parameters::StaticClass();
		case EAGX_LidarModel::OusterOS2:
			return UAGX_OusterOS2Parameters::StaticClass();
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("Unknown Lidar Model passed to FAGX_SensorUtilities::GetParameterTypeFrom. Returning "
			 "nullptr."));
	return nullptr;
}
