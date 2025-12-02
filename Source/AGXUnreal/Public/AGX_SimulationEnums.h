// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"

UENUM()
enum EAGX_StepMode
{
	/** Step the AGX simulation as many times as necessary per Unreal step to keep Unreal and AGX
	   time synchronized at all time. May result in low framerate. */
	SmCatchUpImmediately UMETA(DisplayName = "Catch up immediately"),

	/** Allow up to two AGX simulation steps per Unreal step whenever the AGX simulation lags
	   behind. May result in some stuttering. */
	SmCatchUpOverTime UMETA(DisplayName = "Catch up over time"),

	/** Similar to 'Catch up over time' but will only keep track of time lags smaller or equal to
	   the Time Lag Cap. May result in permanent difference between AGX and Unreal time. */
	SmCatchUpOverTimeCapped UMETA(DisplayName = "Catch up over time Capped"),

	/** Step the AGX simulation up to one time per Unreal step. May result in simulation appearing
	   to run in slow-motion. */
	SmDropImmediately UMETA(DisplayName = "Drop immediately"),

	/** Do not step the AGX Dynamics simulation automatically during tick. Instead call
	   UAGX_Simulation::StepOnce to explicitly step the simulation when needed. */
	SmNone UMETA(DisplayName = "Do not step")
};

UENUM()
enum EAGX_GravityModel
{
	/** Gravity uniform in magnitude over the entire space and directed along a specified vector. */
	Uniform UMETA(DisplayName = "Uniform Gravity Field"),

	/** Gravity uniform in magnitude over the entire space and directed from any given position
	   towards a single point. */
	Point UMETA(DisplayName = "Point Gravity Field")
};
