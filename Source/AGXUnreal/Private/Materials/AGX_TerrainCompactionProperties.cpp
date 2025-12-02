// Copyright 2025, Algoryx Simulation AB.

#include "Materials/AGX_TerrainCompactionProperties.h"

// AGX Dynamics for Unreal includes.
#include "AGX_CustomVersion.h"

void FAGX_TerrainCompactionProperties::Serialize(FArchive& Archive)
{
	Archive.UsingCustomVersion(FAGX_CustomVersion::GUID);
	if (ShouldUpgradeTo(Archive, FAGX_CustomVersion::RenameTerrainMaterialKeNePhi0))
	{
		BankStatePhi0 = Phi0_DEPRECATED;
		HardeningConstantKe = K_e_DEPRECATED;
		HardeningConstantNe = N_e_DEPRECATED;
	}
}
