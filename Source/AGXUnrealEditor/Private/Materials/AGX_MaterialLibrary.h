// Copyright 2023, Algoryx Simulation AB.

#pragma once

namespace AGX_MaterialLibrary
{
	bool InitializeShapeMaterialAssetLibrary();

	/**
	 * Note : Shape Materials must be initialized before calling this function.
	 * I.e. call InitializeShapeMaterialAssetLibrary before calling this function.
	 * This is because the Contact Materials reference Shape Materials.
	 */
	bool InitializeContactMaterialAssetLibrary();

	bool InitializeTerrainMaterialAssetLibrary();
}