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

UENUM()
enum class EAGX_DebuggingMode : uint8
{
	/** Debugging is turned off. */
	None,

	/**
	 * Use Web Debugger, running in a browser.
	 * Open http://localhost:5173/ to see the debugging UI.
	 */
	WebDebugger,

	/**
	 * Remote debugging allows agxViewer, the default scene viewer in AGX
	 * Dynamics, to connect to the AGX_Simulation running inside Unreal Engine
	 * and render the internal simulation state using its built-in debug
	 * rendering capabilities.
	 *
	 * To connect to a running Unreal Engine instance launch agxViewer with
	 *    agxViewer -p --connect localhost:<PORT>
	 * where <PORT> is the port number configured in Project Settings > Plugins >  AGX Dynamics >
	 * Debug > RemoteDebuggingPort.
	 */
	RemoteDebugger,
};
