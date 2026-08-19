// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

/// Mirrors agxSensor::CameraLensSingleElementParameters.
struct AGXUNREALBARRIER_API FCameraLensSingleElementParameters
{
	double focalLength {0.0};
	double fStop {0.0};
	union
	{
		double distance;
		double minimumDistance;
	} focus {0.0};
	bool autofocus {false};
};

