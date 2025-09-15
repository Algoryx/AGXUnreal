// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Misc/Guid.h"

class FArchive;

struct AGXUNREAL_API FAGX_CustomVersion
{
	// Important: Do not remove or change the order of enum literals if those have been released
	// publicly. Doing so will break backward compatibility for anyone using that release.
	// There is an exception: if a backwards compatibility "reset" is wanted, for example when
	// moving to a new major release, all enum literals in this Type enum except the first and the
	// two last ones can be removed. When doing such a "reset", remember to set a newly randomly
	// generated GUID in AGX_CustomVersion.cpp also. That GUID together with these enum literal
	// values are what makes a certain version completely unique.
	enum Type
	{
		// Before any version changes were made.
		BeforeCustomVersionWasAdded = 0,

		ConstraintsStoreComplianceInsteadOfElasticity,

		// To better support large and small number we replace float/double with FAGX_Real in Shape-
		// and ContactMaterial.
		ScientificNotationInMaterials,

		// The ...MaterialBase, ... MaterialInstance, ...MaterialAsset have all been combined into a
		// single class for ShapeMaterials, TerrainMaterials and ContactMaterials respectively.
		BaseAssetInstanceMaterialClassesAreOneClass,

		// Renamed Terrain Material Compaction properties K_e, N_e and Phi0 to more readable names
		// HardeningConstantKe, HardeningConstantNe, BankStatePhi0.
		RenameTerrainMaterialKeNePhi0,

		// Use Height Field Bounds to select samping area of Landscapes in Terrain and
		// HeightFieldShapeComponent.
		HeightFieldUsesBounds,

		// Contact Material Contact Reduction Level type change from UInt8 to enum.
		ContactReductionLevelAsEnum,

		// Terrain, CollisionGroupDisabler and ContactMaterialRegistrar Actors have new root
		// components (viewport icons).
		TerrainCGDisablerCMRegistrarViewporIcons,

		// TerrainMaterial no longer shares a common base class with ShapeMaterial. The Terrain has
		// both a TerrainMaterial and a ShapeMaterial.
		TerrainMaterialShapeMaterialSplit,

		// Use FAGX_Frame for wire route nodes so that they, Body Fixed and Eye, in particular, can
		// be placed relative to a Rigid Body instead of the Wire.
		WireRouteNodeFrame,

		// In Model Source Component, associate render data Static Mesh Components with a Shape
		// instead of a Render Data since the same Render Data can be used by many Shapes.
		RenderDataPerShape,

		// Runtime Import support added, where the Import and Reimport pipeline was refactored
		// completely.
		RuntimeImportSupport,

		// <----- New versions can be added above this line. ----->
		VersionPlusOne,

		LatestVersion = VersionPlusOne - 1
	};

	const static FGuid GUID;

private:
	FAGX_CustomVersion()
	{
	}
};

/**
 * Determine if we are loading from an archive that is older than the given version, which means
 * that we should perform any transformations necessary to bring the state from the pre-version
 * format to the post-version format.
 *
 * @param Archive The archive we're serializing from/to.
 * @param Version The version to compare with.
 * @return True if we should do an upgrade.
 */
bool ShouldUpgradeTo(const FArchive& Archive, FAGX_CustomVersion::Type Version);
