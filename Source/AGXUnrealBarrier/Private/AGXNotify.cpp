// Copyright 2025, Algoryx Simulation AB.

#include "AGXNotify.h"

// AGX Dynamics for Unreal includes.
#include "AGX_LogCategory.h"
#include "AGXUnrealBarrier.h"
#include "TypeConversions.h"

void FAGXNotify::StartAgxNotify(ELogVerbosity::Type LogVerbosity)
{
	agx::Notify::instance()->addCallback(this, ConvertLogLevelVerbosity(LogVerbosity));
}

void FAGXNotify::StopAgxNotify()
{
	agx::Notify::instance()->removeCallback(this);
}

void FAGXNotify::message(const agx::String& msg, int notifyLevel)
{
	const FString Message = Convert(msg);
	ELogVerbosity::Type Verbosity =
		ConvertLogLevelVerbosity((agx::Notify::NotifyLevel) notifyLevel);
	FAGXUnrealBarrierModule::Get().RelayNotifyMessage(Message, Verbosity);

	// Convert log level verbosity from AGX to Unreal.
	// A convert function is not possible to use due to the
	// implementation of the UE_LOG macro.
	//
	// Note: Avoid setting Unreal verbosity level 'Fatal'
	// since this will shut down the editor. AGX's verbosity
	// level 'NOTIFY_ERROR' throws an exception internally
	// and is therefore recoverable.
	switch (notifyLevel)
	{
		case agx::Notify::NOTIFY_DEBUG:
			UE_LOG(LogAGXDynamics, Verbose, TEXT("%s"), *Message);
			break;
		case agx::Notify::NOTIFY_INFO:
			UE_LOG(LogAGXDynamics, Log, TEXT("%s"), *Message);
			break;
		case agx::Notify::NOTIFY_WARNING:
			UE_LOG(LogAGXDynamics, Error, TEXT("%s"), *Message);
			break;
		case agx::Notify::NOTIFY_ERROR:
			UE_LOG(LogAGXDynamics, Error, TEXT("%s"), *Message);
			break;
		default:
			UE_LOG(
				LogAGX, Warning,
				TEXT("AGXNotify::message: unknown notify level: %d. Using verbosity level 'Log' "
					 "for the next log entry:"),
				notifyLevel);

			// Use verbosity level 'Log' by default if unknown notifyLevel is given.
			UE_LOG(LogAGXDynamics, Log, TEXT("%s"), *Message);
	}
}
