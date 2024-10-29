// Copyright 2024, Algoryx Simulation AB.

#pragma once

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include "agx/Referenced.h"
#include "EndAGXIncludes.h"

/**
 * Macro for declaring AGXRef types.
 *
 * @param AGXNamespace The namespace in which the AGX Dynamics type exists.
 * @param AGXType The AGX Dynamics type to declare a Ref type for.
 */
#define AGX_DECLARE_REF(AGXNamespace, AGXType)           \
	struct F##AGXType##Ref                               \
	{                                                    \
		AGXNamespace::AGXType##Ref Native;               \
		F##AGXType##Ref() = default;                     \
		F##AGXType##Ref(AGXNamespace::AGXType* InNative) \
			: Native(InNative)                           \
		{                                                \
		}                                                \
	}
