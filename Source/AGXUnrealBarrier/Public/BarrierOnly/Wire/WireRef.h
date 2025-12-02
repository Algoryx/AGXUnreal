// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "BeginAGXIncludes.h"
#include <agxWire/Wire.h>
#include "EndAGXIncludes.h"

struct FWireRef
{
	using NativeType = agxWire::Wire;
	agxWire::WireRef Native;

	FWireRef() = default;
	FWireRef(agxWire::Wire* InNative)
		: Native(InNative)
	{
	}
};
