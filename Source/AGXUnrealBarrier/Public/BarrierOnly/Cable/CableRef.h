// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "BeginAGXIncludes.h"
#include <agxCable/Cable.h>
#include <agxCable/CableProperties.h>
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

struct FCablePropertiesRef
{
	agxCable::CablePropertiesRef Native;

	FCablePropertiesRef() = default;
	FCablePropertiesRef(agxCable::CableProperties* InNative)
		: Native(InNative)
	{
	}
};

struct FCableRef
{
	agxCable::CableRef Native;

	FCableRef() = default;
	FCableRef(agxCable::Cable* InNative)
		: Native(InNative)
	{
	}
};

