// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "BeginAGXIncludes.h"
#include <agxCollide/RenderData.h>
#include "EndAGXIncludes.h"

struct FRenderDataRef
{
	// ConstRef because agxCollide::Shape only provide pointer-to-const for its
	// render data and that is the primary use case at the moment. Not sure what
	// to do once we need writable Render Data. A second FRenderDataRef class?
	// And rename this with Const somewhere?
	agxCollide::RenderDataConstRef Native;
	FRenderDataRef() = default;
	FRenderDataRef(const agxCollide::RenderData* InNative)
		: Native(InNative)
	{
	}
};
