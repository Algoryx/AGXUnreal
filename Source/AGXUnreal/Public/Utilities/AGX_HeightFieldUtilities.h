// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Shapes/HeightFieldShapeBarrier.h"

// Standard library includes.
#include <tuple>

class ALandscape;

namespace AGX_HeightFieldUtilities
{
	// StartPos is in world coordinate system.
	AGXUNREAL_API FHeightFieldShapeBarrier CreateHeightField(
		ALandscape& Landscape, const FVector& StartPos, double LengthX, double LengthY);

	// Overall resolution using outer bounds (i.e. holes does not affect this value unless a
	// complete part if a side has been removed using the Landscape tool.
	AGXUNREAL_API std::tuple<int32, int32> GetLandscapeNumberOfVertsXY(const ALandscape& Landscape);

	// Size (outer bounds) [cm].
	AGXUNREAL_API std::tuple<double, double> GetLandscapeSizeXY(const ALandscape& Landscape);

	AGXUNREAL_API bool IsOpenWorldLandscape(const ALandscape& Landscape);
}
