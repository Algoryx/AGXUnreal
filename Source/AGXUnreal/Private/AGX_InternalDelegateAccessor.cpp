// Copyright 2025, Algoryx Simulation AB.

#include "AGX_InternalDelegateAccessor.h"

FOnPreStepForwardInternal& FAGX_InternalDelegateAccessor::GetOnPreStepForwardInternal(
	UAGX_Simulation& Simulation)
{
	return Simulation.PreStepForwardInternal;
}

FOnPostStepForwardInternal& FAGX_InternalDelegateAccessor::GetOnPostStepForwardInternal(
	UAGX_Simulation& Simulation)
{
	return Simulation.PostStepForwardInternal;
}
