// Copyright 2022, Algoryx Simulation AB.

#pragma once

#include "BeginAGXIncludes.h"
#include <agxWire/Node.h>
#include "EndAGXIncludes.h"

struct FWireNodeRef
{
	agxWire::NodeRef Native;

	FWireNodeRef() = default;
	FWireNodeRef(agxWire::Node* InNative)
		: Native(InNative)
	{
	}
};
