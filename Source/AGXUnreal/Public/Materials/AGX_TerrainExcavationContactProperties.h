// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include <AGX_Real.h>

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <limits>

#include "AGX_TerrainExcavationContactProperties.generated.h"

/**
 * Struct containing the properties governing some of the contact dynamics between the shovel, soil
 * aggregates and the terrain during excavation and deformation.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_TerrainExcavationContactProperties
{
	GENERATED_BODY()

public:

	bool operator==(const FAGX_TerrainExcavationContactProperties& Other) const = default;

	/**
	 * The contact stiffness multiplier for the generated contacts between the soil
	 * aggregates <-> terrain for excavation and deformation. The final Young's modulus value that
	 * will be used in the contact material thus becomes:
	 * YM_final = BulkYoungsModulus * stiffnessMultiplier
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Excavation Contact")
	FAGX_Real AggregateStiffnessMultiplier {0.002};

	/**
	 * The contact stiffness multiplier for the generated contacts between the soil
	 * aggregates <-> shovels in primary excavation. The final Young's modulus value that will be
	 * used in the contact material thus becomes:
	 * YM_final = BulkYoungsModulus * stiffnessMultiplier.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Excavation Contact")
	FAGX_Real ExcavationStiffnessMultiplier {1.0};

	/**
	 * This determines how rapidly the stored depth in a terrain <-> aggregate contact will decay
	 * during separation when the active zone moves away from the soil aggregate <-> terrain contact
	 * plane.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Excavation Contact")
	FAGX_Real DepthDecayFactor {2.0};

	/**
	 * This governs how fast the depth should increase when the separation direction of the
	 * excavation intersects the contact plane, causing virtual soil compression between soil
	 * aggregates and the terrain.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Excavation Contact")
	FAGX_Real DepthIncreaseFactor {1.0};

	/**
	 * Angle threshold between the separation normal and the terrain-aggregate contact plane.
	 * When the angle is below this threshold, additional depth within the contact plane
	 * contributes to increasing the overall contact depth used in the terrain-aggregate contact
	 * model [deg].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Excavation Contact")
	FAGX_Real DepthAngleThreshold {45.0};

	/**
	 * The maximum force that the soil aggregate <-> terrain contacts are allowed to have. Default
	 * maximum values are determined by the soil mechanics properties of the terrain [N].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Excavation Contact")
	FAGX_Real MaximumAggregateNormalForce {std::numeric_limits<double>::infinity()};

	/**
	 * The maximum depth of a soil aggregate <-> terrain contact. This increases when the
	 * separation direction of the excavation intersects the contact plane, causing virtual soil
	 * compression between soil aggregates and the terrain [cm].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Excavation Contact")
	FAGX_Real MaximumContactDepth {100.0};
};
