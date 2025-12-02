// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Misc/AssertionMacros.h"

#if defined(AGXUNREAL_CHECK) && AGXUNREAL_CHECK == 1
#define AGX_CHECK(expr) check(expr)
#define AGX_CHECKF(expr, format,  ...) checkf(expr, format, ##__VA_ARGS__)
#else
#define AGX_CHECK(x)
#define AGX_CHECKF(exprt, format, ...)
#endif
