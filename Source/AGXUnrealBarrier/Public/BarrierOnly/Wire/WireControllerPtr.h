// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include "agxWire/WireController.h"
#include "EndAGXIncludes.h"

struct FWireControllerPtr
{
	agxWire::WireController* Native = nullptr;
	FWireControllerPtr() = default;
	FWireControllerPtr(agxWire::WireController* InNative)
		: Native(InNative)
	{
	}
};
