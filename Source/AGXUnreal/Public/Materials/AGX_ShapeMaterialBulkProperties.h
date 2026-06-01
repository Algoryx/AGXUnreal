// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include <AGX_Real.h>

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "AGX_ShapeMaterialBulkProperties.generated.h"

/**
 * Physical properties for the bulk of Shapes using the AGX Material.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ShapeMaterialBulkProperties
{
	GENERATED_BODY()

public:

	bool operator==(const FAGX_ShapeMaterialBulkProperties& Other) const = default;

	/**
	 * Density of Shapes using the material [kg/m^3].
	 * The density can be used for automatic calculation of total mass and inertia of the Rigid Body
	 * (see mass options of Rigid Body Component).
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Material Bulk Properties",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real Density;

	/**
	 * Young's modulus of the material, same as spring coefficient k [Pa].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Material Bulk Properties",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real YoungsModulus;

	/**
	 * Bulk viscosity coefficient of the material (1.0 - restitution coefficient).
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Material Bulk Properties",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real Viscosity;

	/**
	 * Spook Damping used by the contact constraint [s].
	 * The value is the time the contact constraint has to fulfill its violation.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Material Bulk Properties",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real SpookDamping;

	/**
	 * Minimum elastic rest length of the contact material [cm].
	 *
	 * This is only used if the contact area approach is used if the 'Use Contact Area Approach' is
	 * enabled.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Material Bulk Properties")
	FAGX_Real MinElasticRestLength;

	/**
	 * Maximum elastic rest length of the contact material [cm].
	 *
	 * This is only used if the contact area approach is used if the 'Use Contact Area Approach' is
	 * enabled.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Material Bulk Properties")
	FAGX_Real MaxElasticRestLength;

public:
	FAGX_ShapeMaterialBulkProperties();
};
