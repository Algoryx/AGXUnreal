// Copyright 2024, Algoryx Simulation AB.

#pragma once

namespace AGX_MaterialLibrary
{
	bool InitializeShapeMaterialAssetLibrary(bool ForceOverwrite = false);

	/**
	 * Note : Shape Materials must be initialized before calling this function.
	 * I.e. call InitializeShapeMaterialAssetLibrary before calling this function.
	 * This is because the Contact Materials reference Shape Materials.
	 */
	bool InitializeContactMaterialAssetLibrary(bool ForceOverwrite = false);

	bool InitializeTerrainMaterialAssetLibrary(bool ForceOverwrite = false);

	bool InitializeLidarAmbientMaterialAssetLibrary(bool ForceOverwrite = false);
}
