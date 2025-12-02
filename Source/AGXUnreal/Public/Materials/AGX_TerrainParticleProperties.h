// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include <AGX_Real.h>

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_TerrainParticleProperties.generated.h"

/**
 * Struct containing the properties of the created soil particles in the Terrain, such as particle
 * density and contact parameters amongst soil particles and between soil particles and the terrain
 * surface.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_TerrainParticleProperties
{
	GENERATED_BODY()

public:

	bool operator==(const FAGX_TerrainParticleProperties& Other) const = default;

	/**
	 * The adhesion overlap factor of particle contacts, i.e what fraction of the particle
	 * radius is allowed to overlap to simulate adhesion.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Particle")
	FAGX_Real AdhesionOverlapFactor {0.05};

	/**
	 * The particle cohesion of the bulk material [Pa].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Particle")
	FAGX_Real ParticleCohesion {0.0};

	/**
	 * The particle restitution of the bulk material.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Particle")
	FAGX_Real ParticleRestitution {0.5};

	/**
	 * The particle rolling resistance of the bulk material.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Particle")
	FAGX_Real ParticleRollingResistance {0.3};

	/**
	 * The particle surface friction of the bulk material.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Particle")
	FAGX_Real ParticleSurfaceFriction {0.4};

	/**
	 * The particle <-> terrain cohesion of the bulk material [Pa].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Particle")
	FAGX_Real ParticleTerrainCohesion {0.0};

	/**
	 * The particle <-> terrain restitution of the bulk material.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Particle")
	FAGX_Real ParticleTerrainRestitution {0.0};

	/**
	 * The particle <-> terrain rolling resistance of the bulk material.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Particle")
	FAGX_Real ParticleTerrainRollingResistance {0.3};

	/**
	 * The particle <-> terrain surface friction of the bulk material.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Particle")
	FAGX_Real ParticleTerrainSurfaceFriction {0.8};

	/**
	 * The particle <-> Terrain Young's modulus of the bulk material [Pa].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Particle")
	FAGX_Real ParticleTerrainYoungsModulus {1e8};

	/**
	 * The particle Young's modulus of the bulk material [Pa].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Material Particle")
	FAGX_Real ParticleYoungsModulus {1e9};
};
