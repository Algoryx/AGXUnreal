// Copyright 2025, Algoryx Simulation AB.

#include "Materials/AGX_TerrainBulkProperties.h"

FAGX_TerrainBulkProperties::FAGX_TerrainBulkProperties()
	: AdhesionOverlapFactor(0.05)
	, Cohesion(0.0)
	, Density(1400.0)
	, DilatancyAngle(15.0)
	, FrictionAngle(45.0)
	, MaxDensity(1600.0)
	, PoissonsRatio(0.1)
	, SwellFactor(1.1)
	, YoungsModulus(1.0e7)
{
}
