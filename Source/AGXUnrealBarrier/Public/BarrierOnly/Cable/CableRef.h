// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "BeginAGXIncludes.h"
#include <agxCable/Cable.h>
#include "EndAGXIncludes.h"

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
