// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "Logging/LogMacros.h"

/**
 * Declares AGX specific Unreal log category.
 *
 * Default verbosity is set to 'Verbose', and will be used when one is not specified in the
 * ini files or on the command line. Anything more verbose than this will not be logged.
 * The reason for setting 'Verbose' as default instead of e.g. 'Log' is to make it possible
 * to get the lowest level (NOTIFY_DEBUG) log messages from AGX.
 *
 * Compile time verbosity is set to 'All'.
 */
AGXUNREALBARRIER_API DECLARE_LOG_CATEGORY_EXTERN(LogAGX, Verbose, All);

/// Log category for messages forwarded from AGX Dynamics.
AGXUNREALBARRIER_API DECLARE_LOG_CATEGORY_EXTERN(LogAGXDynamics, Verbose, All);
