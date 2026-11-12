// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "Shapes/HeightFieldShapeBarrier.h"

// Standard library includes.
#include <tuple>

class ALandscape;

namespace AGX_HeightFieldUtilities
{
	/**
	 * Source heights in Landscape-local centimeters and AGX row order. StartPos is world space.
	 * Spacing [cm] overrides the sampling grid in both axes; zero uses the Landscape spacing.
	 */
	AGXUNREAL_API TArray<float> GetHeights(
		ALandscape& Landscape, const FVector& StartPos, double LengthX, double LengthY,
		double Spacing = 0.0);

	/**
	 * StartPos is world space. Positive Spacing [cm] creates a uniform grid fitted inside the
	 * requested lengths, anchored at StartPos. Zero preserves the Landscape's quad spacing.
	 */
	AGXUNREAL_API FHeightFieldShapeBarrier CreateHeightField(
		ALandscape& Landscape, const FVector& StartPos, double LengthX, double LengthY,
		double Spacing = 0.0);

	// Overall resolution using outer bounds (i.e. holes does not affect this value unless a
	// complete part if a side has been removed using the Landscape tool.
	AGXUNREAL_API std::tuple<int32, int32> GetLandscapeNumberOfVertsXY(const ALandscape& Landscape);

	// Size (outer bounds) [cm].
	AGXUNREAL_API std::tuple<double, double> GetLandscapeSizeXY(const ALandscape& Landscape);

	AGXUNREAL_API bool IsOpenWorldLandscape(const ALandscape& Landscape);
}
