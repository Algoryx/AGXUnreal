// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_TerrainTerramechanicsProperties.generated.h"

/**
 * Terrain Terramechanics Properties that affects the phyiscal behaviour of e.g. Terrain Wheels when
 * contacting the Terrain.
 */
USTRUCT()
struct AGXUNREAL_API FAGX_TerrainTerramechanicsProperties
{
	GENERATED_BODY()

public:
	bool operator==(const FAGX_TerrainTerramechanicsProperties& Other) const = default;

	/**
	 * The 'n_0' in n = n_0 + n_1 * slip_ratio, where 'n' is sinkage exponent.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Terramechanics Properties")
	double SinkageExponentParameterA {1.0};

	/**
	 * The 'n_1' in n = n_0 + n_1 * slip_ratio, where 'n' is sinkage exponent.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Terramechanics Properties")
	double SinkageExponentParameterB {0.0};

	/**
	 * The 'a_0' in K_x = a_0 + slip_angle * a_1, where
	 * tau_x = [c + sigma*tan(phi)][1-exp(-j_x/K_x)].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Terramechanics Properties")
	double ShearModulusXParameterA {0.036};

	/**
	 * The 'a_1' in K_x = a_0 + slip_angle * a_1.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Terramechanics Properties")
	double ShearModulusXParameterB {0.043};

	/**
	 * The 'a_0' in K_y = a_0 + slip_angle * a_1, where
	 * tau_y = [c + sigma*tan(phi)][1-exp(-j_y/K_y)].
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Terramechanics Properties")
	double ShearModulusYParameterA {0.013};

	/**
	 * The 'a_1' in K_y = a_0 + slip_angle * a_1.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Terramechanics Properties")
	double ShearModulusYParameterB {0.020};

	/**
	 * The 'k_c' in p = (k_c/b + k_phi)z^n.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Terramechanics Properties")
	double CohesiveModulusBekker {1000.0};

	/**
	 * The 'k_phi' in p = (k_c/b + k_phi)z^n.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Terramechanics Properties")
	double FrictionalModulusBekker {800000.0};

	/**
	 * The 'k_c_prime' in
	 * p = (c*k_c_prime + rho*g*b*k_phi_prime)(z/b)^n.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Terramechanics Properties")
	double CohesiveModulusReece {670.0};

	/**
	 * The 'k_phi_prime' in
	 * p = (c*k_c_prime + rho*g*b*k_phi_prime)(z/b)^n.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Terramechanics Properties")
	double FrictionalModulusReece {200.0};

	/**
	 * The 'a_0' in theta_m = (a_0 + a_1*slip_ratio) * theta_f.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Terramechanics Properties")
	double MaximumNormalStressAngleParameterA {0.4};

	/**
	 * The 'a_1' in theta_m = (a_0 + a_1*slip_ratio) * theta_f.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Terramechanics Properties")
	double MaximumNormalStressAngleParameterB {0.15};

	/**
	 * The 'a_0' in theta_r = (a_0 + a_1*slip_ratio) * theta_f.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Terramechanics Properties")
	double RearAngleParameterA {-0.12};

	/**
	 * The 'a_1' in theta_r = (a_0 + a_1*slip_ratio) * theta_f.
	 */
	UPROPERTY(EditAnywhere, Category = "AGX Terrain Terramechanics Properties")
	double RearAngleParameterB {0.0};
};
