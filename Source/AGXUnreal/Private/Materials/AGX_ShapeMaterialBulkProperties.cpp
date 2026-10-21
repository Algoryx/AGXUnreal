// Copyright 2026, Algoryx Simulation AB.

#include "Materials/AGX_ShapeMaterialBulkProperties.h"

// AGX Dynamics for Unreal includes.
#include "AGX_CustomVersion.h"
#include "AGX_Simulation.h"
#include "Utilities/AGXUtilities.h"

// Unreal Engine includes.
#include "Serialization/Archive.h"
#include "UObject/UObjectGlobals.h"

FAGX_ShapeMaterialBulkProperties::FAGX_ShapeMaterialBulkProperties()
	: Density(1000.0)
	, YoungsModulus(2.0 / 5.0E-9)
	, Viscosity(0.5)
	, Attenuation(4.5)
	, MinElasticRestLength(0.05)
	, MaxElasticRestLength(5.0)
	, SpookDamping_DEPRECATED(4.5 / 60.0)
{
}

bool FAGX_ShapeMaterialBulkProperties::operator==(
	const FAGX_ShapeMaterialBulkProperties& Other) const
{
	return Density == Other.Density && YoungsModulus == Other.YoungsModulus &&
		   Viscosity == Other.Viscosity && Attenuation == Other.Attenuation &&
		   MinElasticRestLength == Other.MinElasticRestLength &&
		   MaxElasticRestLength == Other.MaxElasticRestLength;
}

void FAGX_ShapeMaterialBulkProperties::Serialize(FArchive& Archive)
{
	Archive.UsingCustomVersion(FAGX_CustomVersion::GUID);
	if (ShouldUpgradeTo(Archive, FAGX_CustomVersion::DampingToAttenuation))
	{
		Attenuation = FAGXUtilities::ConvertSpookDampingToAttenuation(
			SpookDamping_DEPRECATED, GetDefault<UAGX_Simulation>()->TimeStep);
	}
}
