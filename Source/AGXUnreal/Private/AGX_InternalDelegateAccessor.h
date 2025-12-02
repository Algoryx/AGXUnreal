// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "AGX_Simulation.h"

/**
 * Internal convenience class with the purpose of allowing AGX Dynamics for Unreal classes to access
 * internal delegates which should not be reachable from user code.
 */
class FAGX_InternalDelegateAccessor
{
public:
	static FOnPreStepForwardInternal& GetOnPreStepForwardInternal(UAGX_Simulation& Simulation);
	static FOnPostStepForwardInternal& GetOnPostStepForwardInternal(UAGX_Simulation& Simulation);
};
