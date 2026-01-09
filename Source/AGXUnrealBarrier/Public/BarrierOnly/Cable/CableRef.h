// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "BeginAGXIncludes.h"
#include <agxCable/Cable.h>
#include <agxCable/Node.h>
#include "EndAGXIncludes.h"

struct FCableNodeRef
{
	agxCable::RoutingNodeRef Native;

	FCableNodeRef() = default;
	FCableNodeRef(agxCable::RoutingNode* InNative)
		: Native(InNative)
	{
	}
};

struct FCableRef
{
	using NativeType = agxCable::Cable;
	agxCable::CableRef Native;

	FCableRef() = default;
	FCableRef(agxCable::Cable* InNative)
		: Native(InNative)
	{
	}
};

