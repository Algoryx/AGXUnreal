// Copyright 2025, Algoryx Simulation AB.

#include "Materials/AGX_ContactMaterialMechanicsApproach.h"

FAGX_ContactMaterialMechanicsApproach::FAGX_ContactMaterialMechanicsApproach()
	: bUseContactAreaApproach(false)
	, MinElasticRestLength(2.0 * 0.05)
	, MaxElasticRestLength(2.0 * 5)
{
	// See agx\src\agx\Material.cpp for default values
}
