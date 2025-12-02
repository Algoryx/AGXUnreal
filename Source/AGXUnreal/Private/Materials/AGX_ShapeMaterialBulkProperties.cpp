// Copyright 2025, Algoryx Simulation AB.

#include "Materials/AGX_ShapeMaterialBulkProperties.h"

FAGX_ShapeMaterialBulkProperties::FAGX_ShapeMaterialBulkProperties()
	: Density(1000.0)
	, YoungsModulus(2.0 / 5.0E-9)
	, Viscosity(0.5)
	, SpookDamping(4.5 / 60.0)
	, MinElasticRestLength(0.05)
	, MaxElasticRestLength(5.0)
{
}
