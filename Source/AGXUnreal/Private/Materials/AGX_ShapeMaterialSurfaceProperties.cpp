// Copyright 2026, Algoryx Simulation AB.

#include "Materials/AGX_ShapeMaterialSurfaceProperties.h"

// AGX Dynamics for Unreal includes.
#include "AGX_CustomVersion.h"

// Unreal Engine includes.
#include "Serialization/Archive.h"

FAGX_ShapeMaterialSurfaceProperties::FAGX_ShapeMaterialSurfaceProperties()
	: bFrictionEnabled(true)
	, Roughness(0.25 / (2 * 0.3))
	, FrictionCompliance(5.0e-9)
	, /// \todo This value is too small for Unreal's default UI slider! Make our own!
	AdhesiveForce(0.0)
	, AdhesiveOverlap(0.0)
	, Viscosity_DEPRECATED(2.0 / (2.0 / 5.0E-9))
{
}

bool FAGX_ShapeMaterialSurfaceProperties::operator==(
	const FAGX_ShapeMaterialSurfaceProperties& Other) const
{
	return bFrictionEnabled == Other.bFrictionEnabled && Roughness == Other.Roughness &&
		   FrictionCompliance == Other.FrictionCompliance && AdhesiveForce == Other.AdhesiveForce &&
		   AdhesiveOverlap == Other.AdhesiveOverlap;
}

void FAGX_ShapeMaterialSurfaceProperties::Serialize(FArchive& Archive)
{
	Archive.UsingCustomVersion(FAGX_CustomVersion::GUID);
	if (ShouldUpgradeTo(Archive, FAGX_CustomVersion::DampingToAttenuation))
		FrictionCompliance = Viscosity_DEPRECATED;
}
