// Copyright 2026, Algoryx Simulation AB.

#include "Utilities/AGX_SensorUtilities.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "Sensors/AGX_CustomRayPatternParameters.h"
#include "Sensors/AGX_GenericHorizontalSweepParameters.h"
#include "Sensors/AGX_LivoxAviaParameters.h"
#include "Sensors/AGX_LivoxHAPParameters.h"
#include "Sensors/AGX_LivoxHorizonParameters.h"
#include "Sensors/AGX_LivoxMid360Parameters.h"
#include "Sensors/AGX_LivoxMid40Parameters.h"
#include "Sensors/AGX_LivoxMid70Parameters.h"
#include "Sensors/AGX_LivoxTeleParameters.h"
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
		case EAGX_LidarModel::LivoxAvia:
			return UAGX_LivoxAviaParameters::StaticClass();
		case EAGX_LidarModel::LivoxHAP:
			return UAGX_LivoxHAPParameters::StaticClass();
		case EAGX_LidarModel::LivoxHorizon:
			return UAGX_LivoxHorizonParameters::StaticClass();
		case EAGX_LidarModel::LivoxMid40:
			return UAGX_LivoxMid40Parameters::StaticClass();
		case EAGX_LidarModel::LivoxMid70:
			return UAGX_LivoxMid70Parameters::StaticClass();
		case EAGX_LidarModel::LivoxMid360:
			return UAGX_LivoxMid360Parameters::StaticClass();
		case EAGX_LidarModel::LivoxTele:
			return UAGX_LivoxTeleParameters::StaticClass();
		case EAGX_LidarModel::Invalid:
			return nullptr;
	}

	UE_LOG(
		LogAGX, Error,
		TEXT("Unknown Lidar Model passed to FAGX_SensorUtilities::GetParameterTypeFrom. Returning "
			 "nullptr."));
	return nullptr;
}
