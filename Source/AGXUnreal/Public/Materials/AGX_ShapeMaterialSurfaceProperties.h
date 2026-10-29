// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include <AGX_Real.h>

// Unreal Engine includes
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AGX_ShapeMaterialSurfaceProperties.generated.h"

/**
 * Physical properties for the surface of Shapes using the AGX Material.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_ShapeMaterialSurfaceProperties
{
	GENERATED_BODY()

public:
	bool operator==(const FAGX_ShapeMaterialSurfaceProperties& Other) const;

	/**
	 * Specify if the friction should be used when solving contacts for this Material.
	 * If this is set to false, the solver will NOT calculate any friction when this material
	 * is in contact with another material.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Material Surface Properties")
	bool bFrictionEnabled;

	/**
	 * Unitless roughness parameter used to calculate the final friction coefficient.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Material Surface Properties",
		Meta = (ClampMin = "0.0", UIMin = "0.0", EditCondition = "bFrictionEnabled"))
	FAGX_Real Roughness;

	/**
	 * Friction compliance controlling slip and creep between contacting surfaces.
	 * Larger values allow more creep.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Material Surface Properties",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real FrictionCompliance;

	/**
	 * The attractive force between two colliding objects [N].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Material Surface Properties",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real AdhesiveForce;

	/**
	 * Allowed overlap from surface for resting contact [cm].
	 *
	 * At lower overlap, the adhesion force will take effect.
	 * At this overlap, no adhesive force is applied.
	 * At higher overlap, the (usual) contact force is applied.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Material Surface Properties",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real AdhesiveOverlap;

public:
	FAGX_ShapeMaterialSurfaceProperties();
	void Serialize(FArchive& Archive);

private:
	UPROPERTY()
	FAGX_Real Viscosity_DEPRECATED;
};
