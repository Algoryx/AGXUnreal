// Copyright 2025, Algoryx Simulation AB.

#include "AGX_Edge.h"

FTwoVectors FAGX_Edge::GetLocationsRelativeTo(const USceneComponent& Component) const
{
	FTwoVectors Line;
	Line.v1 = Start.GetLocationRelativeTo(Component);
	Line.v2 = End.GetLocationRelativeTo(Component);
	return Line;
}

FTwoVectors FAGX_Edge::GetLocationsRelativeTo(
	const USceneComponent& Component, const USceneComponent& FallbackParent) const
{
	FTwoVectors Line;
	Line.v1 = Start.GetLocationRelativeTo(Component, FallbackParent);
	Line.v2 = End.GetLocationRelativeTo(Component, FallbackParent);
	return Line;
}
