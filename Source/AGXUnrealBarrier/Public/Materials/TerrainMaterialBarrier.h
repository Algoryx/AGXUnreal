// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "Containers/UnrealString.h"

// Standard library includes.
#include <memory>

struct FTerrainMaterialRef;

/**
 * Acts as an interface to a native AGX Terrain Material, and encapsulates it so that it is
 * completely hidden from code that includes this file.
 */
class AGXUNREALBARRIER_API FTerrainMaterialBarrier
{
public:
	FTerrainMaterialBarrier();
	FTerrainMaterialBarrier(FTerrainMaterialBarrier&& Other);
	FTerrainMaterialBarrier(std::unique_ptr<FTerrainMaterialRef> Native);
	virtual ~FTerrainMaterialBarrier();

	bool HasNative() const;
	FTerrainMaterialRef* GetNative();
	const FTerrainMaterialRef* GetNative() const;

	void AllocateNative(const FString& Name);
	void ReleaseNative();

	void SetName(const FString& Name);
	FString GetName() const;

	// Bulk properties.
	void SetAdhesionOverlapFactor(double AdhesionOverlapFactor);
	double GetAdhesionOverlapFactor() const;

	void SetCohesion(double Cohesion);
	double GetCohesion() const;

	void SetDensity(double Density);
	double GetDensity() const;

	void SetDilatancyAngle(double DilatancyAngle);
	double GetDilatancyAngle() const;

	void SetFrictionAngle(double FrictionAngle);
	double GetFrictionAngle() const;

	void SetMaxDensity(double MaxDensity);
	double GetMaxDensity() const;

	void SetPoissonsRatio(double PoissonsRatio);
	double GetPoissonsRatio() const;

	void SetSwellFactor(double SwellFactor);
	double GetSwellFactor() const;

	void SetYoungsModulus(double YoungsModulus);
	double GetYoungsModulus() const;

	// Compaction properties.
	void SetAngleOfReposeCompactionRate(double AngleOfReposeCompactionRate);
	double GetAngleOfReposeCompactionRate() const;

	void SetBankStatePhi(double Phi0);
	double GetBankStatePhi() const;

	void SetCompactionTimeRelaxationConstant(double CompactionTimeRelaxationConstant);
	double GetCompactionTimeRelaxationConstant() const;

	void SetCompressionIndex(double CompressionIndex);
	double GetCompressionIndex() const;

	void SetHardeningConstantKe(double K_e);
	double GetHardeningConstantKe() const;

	void SetHardeningConstantNe(double N_e);
	double GetHardeningConstantNe() const;

	void SetPreconsolidationStress(double PreconsolidationStress);
	double GetPreconsolidationStress() const;

	void SetStressCutOffFraction(double StressCutOffFraction);
	double GetStressCutOffFraction() const;

	void SetDilatancyAngleScalingFactor(double DilatancyAngleScalingFactor);
	double GetDilatancyAngleScalingFactor() const;

	// Particle properties.
	void SetParticleAdhesionOverlapFactor(double ParticleAdhesionOverlapFactor);
	double GetParticleAdhesionOverlapFactor() const;

	void SetParticleCohesion(double ParticleCohesion);
	double GetParticleCohesion() const;

	void SetParticleRestitution(double ParticleRestitution);
	double GetParticleRestitution() const;

	void SetParticleRollingResistance(double ParticleRollingResistance);
	double GetParticleRollingResistance() const;

	void SetParticleSurfaceFriction(double ParticleSurfaceFriction);
	double GetParticleSurfaceFriction() const;

	void SetParticleTerrainCohesion(double ParticleTerrainCohesion);
	double GetParticleTerrainCohesion() const;

	void SetParticleTerrainRestitution(double ParticleTerrainRestitution);
	double GetParticleTerrainRestitution() const;

	void SetParticleTerrainRollingResistance(double ParticleTerrainRollingResistance);
	double GetParticleTerrainRollingResistance() const;

	void SetParticleTerrainSurfaceFriction(double ParticleTerrainSurfaceFriction);
	double GetParticleTerrainSurfaceFriction() const;

	void SetParticleTerrainYoungsModulus(double ParticleTerrainYoungsModulus);
	double GetParticleTerrainYoungsModulus() const;

	void SetParticleYoungsModulus(double ParticleYoungsModulus);
	double GetParticleYoungsModulus() const;

	// Excavation contact properties.
	void SetAggregateStiffnessMultiplier(double AggregateStiffnessMultiplier);
	double GetAggregateStiffnessMultiplier() const;

	void SetExcavationStiffnessMultiplier(double ExcavationStiffnessMultiplier);
	double GetExcavationStiffnessMultiplier() const;

	void SetDepthDecayFactor(double DepthDecayFactor);
	double GetDepthDecayFactor() const;

	void SetDepthIncreaseFactor(double DepthIncreaseFactor);
	double GetDepthIncreaseFactor() const;
	
	void SetDepthAngleThreshold(double DepthAngleThreshold);
	double GetDepthAngleThreshold() const;

	void SetMaximumAggregateNormalForce(double MaximumAggregateNormalForce);
	double GetMaximumAggregateNormalForce() const;

	void SetMaximumContactDepth(double MaximumContactDepth);
	double GetMaximumContactDepth() const;

private:
	FTerrainMaterialBarrier(const FTerrainMaterialBarrier&) = delete;
	void operator=(const FTerrainMaterialBarrier&) = delete;

	// NativeRef has the same lifetime as this object, so it should never be null.
	// NativeRef->Native is created by AllocateNative(), released by ReleaseNative(), and can be
	// null.
	std::unique_ptr<FTerrainMaterialRef> NativeRef;
};
