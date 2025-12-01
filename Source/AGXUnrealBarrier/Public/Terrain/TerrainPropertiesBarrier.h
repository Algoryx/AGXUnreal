// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

#include "TerrainPropertiesBarrier.generated.h"

struct FTerrainPropertiesRef;

USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FTerrainPropertiesBarrier
{
	GENERATED_BODY()

	FTerrainPropertiesBarrier();
	FTerrainPropertiesBarrier(std::shared_ptr<FTerrainPropertiesRef> Native);

	void SetCreateParticles(bool CreateParticles);
	bool GetCreateParticles() const;

	void SetDeleteParticlesOutsideBounds(bool DeleteParticlesOutsideBounds);
	bool GetDeleteParticlesOutsideBounds() const;

	void SetPenetrationForceVelocityScaling(double PenetrationForceVelocityScaling);
	double GetPenetrationForceVelocityScaling() const;

	void SetMaximumParticleActivationVolume(double MaximumParticleActivationVolume);
	double GetMaximumParticleActivationVolume() const;

	void SetSoilParticleSizeScaling(float Scaling);
	float GetSoilParticleSizeScaling() const;

	void SetSoilParticleGrowthRate(double InRate);
	double GetSoilParticleGrowthRate() const;

	void SetSoilParticleMergeRate(double InRate);
	double GetSoilParticleMergeRate() const;

	void SetSoilParticleMergeThreshold(double InThreshold);
	double GetSoilParticleMergeThreshold() const;

	void SetSoilMergeSpeedThreshold(double InThreshold);
	double GetSoilMergeSpeedThreshold() const;

	void SetSoilParticleLifeTime(double InLifeTime);
	double GetSoilParticleLifeTime() const;

	void SetEnableAvalanching(bool Enable);
	bool GetEnableAvalanching() const;

	void SetAvalancheMaxHeightGrowth(double InValue);
	double GetAvalancheMaxHeightGrowth() const;

	void SetAvalancheDecayFraction(double InValue);
	double GetAvalancheDecayFraction() const;

	void SetAvalancheErrorThreshold(double InValue);
	double GetAvalancheErrorThreshold() const;

	void SetActivationSpeed(double InSpeed);
	double GetActivationSpeed() const;

	void SetEnableDeformation(bool bEnable);
	bool GetEnableDeformation() const;

	void SetEnableLockedBorders(bool Enable);
	bool GetEnableLockedBorders() const;

	void SetEnableSoilCompaction(bool Enable);
	bool GetEnableSoilCompaction() const;

	bool HasNative() const;
	void AllocateNative();
	FTerrainPropertiesRef* GetNative();
	const FTerrainPropertiesRef* GetNative() const;

	void ReleaseNative();

private:
	std::shared_ptr<FTerrainPropertiesRef> NativeRef;
};
