// Copyright 2026, Algoryx Simulation AB.

#pragma once

#include "BeginAGXIncludes.h"
#include <agxWire/Link.h>
#include "EndAGXIncludes.h"

struct FWireLinkRef
{
	agxWire::LinkRef Native;

	FWireLinkRef() = default;
	FWireLinkRef(agxWire::Link* InNative)
		: Native(InNative)
	{
	}
};

