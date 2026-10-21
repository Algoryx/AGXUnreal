// Copyright 2026, Algoryx Simulation AB.

#include "Materials/AGX_ShapeMaterialWireProperties.h"

// AGX Dynamics for Unreal includes.
#include "AGX_CustomVersion.h"
#include "AGX_Simulation.h"
#include "Utilities/AGXUtilities.h"

// Unreal Engine includes.
#include "Serialization/Archive.h"
#include "UObject/UObjectGlobals.h"

FAGX_ShapeMaterialWireProperties::FAGX_ShapeMaterialWireProperties()
	: YoungsModulusStretch(6E10)
	, AttenuationStretch(3.0)
	, YoungsModulusBend(6E10)
	, AttenuationBend(6.0)
	, SpookDampingStretch_DEPRECATED(0.05)
	, SpookDampingBend_DEPRECATED(0.10)
{
}

bool FAGX_ShapeMaterialWireProperties::operator==(
	const FAGX_ShapeMaterialWireProperties& Other) const
{
	return YoungsModulusStretch == Other.YoungsModulusStretch &&
		   AttenuationStretch == Other.AttenuationStretch &&
		   YoungsModulusBend == Other.YoungsModulusBend && AttenuationBend == Other.AttenuationBend;
}

void FAGX_ShapeMaterialWireProperties::Serialize(FArchive& Archive)
{
	Archive.UsingCustomVersion(FAGX_CustomVersion::GUID);
	if (ShouldUpgradeTo(Archive, FAGX_CustomVersion::DampingToAttenuation))
	{
		const double TimeStep = GetDefault<UAGX_Simulation>()->TimeStep;
		AttenuationStretch = FAGXUtilities::ConvertSpookDampingToAttenuation(
			SpookDampingStretch_DEPRECATED, TimeStep);
		AttenuationBend =
			FAGXUtilities::ConvertSpookDampingToAttenuation(SpookDampingBend_DEPRECATED, TimeStep);
	}
}
