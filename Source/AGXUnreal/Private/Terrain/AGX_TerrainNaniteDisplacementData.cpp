// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/AGX_TerrainNaniteDisplacementData.h"

// Unreal Engine includes.
#include "Engine/StaticMesh.h"

bool UAGX_TerrainNaniteDisplacementData::IsCurrent(const UStaticMesh& Mesh) const
{
	if (Version != CurrentVersion)
		return false;
#if WITH_EDITORONLY_DATA
	return MeshLightingGuid == Mesh.GetLightingGuid();
#else
	// Stock UE returns a null lighting GUID without editor data. The versioned marker
	// belongs to this cooked mesh; a stock Landscape rebuild creates an unmarked mesh.
	return true;
#endif
}
