#include "Materials/AGX_ShapeMaterialBulkProperties.h"

FAGX_ShapeMaterialBulkProperties::FAGX_ShapeMaterialBulkProperties()
	: Density(1000.0)
	, YoungsModulus(2.0 / 5.0E-9)
	, Viscosity(0.5)
	, Damping(4.5 / 60.0)
	, MinElasticRestLength(0.0005)
	, MaxElasticRestLength(0.05)
{
}