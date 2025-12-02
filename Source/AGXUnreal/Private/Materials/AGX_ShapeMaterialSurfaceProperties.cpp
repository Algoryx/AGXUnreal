// Copyright 2025, Algoryx Simulation AB.

#include "Materials/AGX_ShapeMaterialSurfaceProperties.h"

FAGX_ShapeMaterialSurfaceProperties::FAGX_ShapeMaterialSurfaceProperties()
	: bFrictionEnabled(true)
	, Roughness(0.25 / (2 * 0.3))
	, Viscosity(2.0 / (2.0 / 5.0E-9))
	, /// \todo This value is too small for Unreal's default UI slider! Make our own!
	AdhesiveForce(0.0)
	, AdhesiveOverlap(0.0)
{
}
