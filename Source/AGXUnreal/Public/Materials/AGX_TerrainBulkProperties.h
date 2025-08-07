// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include <AGX_Real.h>

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_TerrainBulkProperties.generated.h"

/**
 * Terrain specific physical properties.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_TerrainBulkProperties
{
	GENERATED_BODY()

public:

	bool operator==(const FAGX_TerrainBulkProperties& Other) const = default;

	/**
	 * Sets the adhesion overlap factor of the bulk material, i.e what fraction of the particle
	 * radius is allowed to overlap to simulate adhesion.
	 *
	 * As of AGX Dynamics versions 2.29 this property is deprecated. The the property with the same
	 * name in Terrain Particle Properties instead.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Material Bulk",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real AdhesionOverlapFactor;

	/**
	 * Sets the bulk cohesion of the bulk material [Pa].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Material Bulk",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real Cohesion;

	/**
	 * Density of the terrain bulk [kg/m^3].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Material Bulk",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real Density;

	/**
	 * Sets the dilatancy angle of the bulk material [deg].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Bulk")
	FAGX_Real DilatancyAngle;

	/**
	 * Sets the internal friction angle of the bulk material [deg].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Bulk")
	FAGX_Real FrictionAngle;

	/**
	 * Sets the maximum density of the bulk material [kg/m^3].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Material Bulk",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real MaxDensity;

	/**
	 * Sets the Poisson's ratio of the bulk material.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Material Bulk",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real PoissonsRatio;

	/**
	 * Sets the swell factor of the material, i.e how much the material will expand during
	 * excavation.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Material Bulk",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real SwellFactor;

	/**
	 * Sets the bulk Young's modulus of the bulk material [Pa].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Terrain Material Bulk",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real YoungsModulus;

public:
	FAGX_TerrainBulkProperties();
};
