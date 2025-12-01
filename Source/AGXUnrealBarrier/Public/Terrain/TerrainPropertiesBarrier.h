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

	bool HasNative() const;
	void AllocateNative();
	FTerrainPropertiesRef* GetNative();
	const FTerrainPropertiesRef* GetNative() const;

	void ReleaseNative();

private:
	std::shared_ptr<FTerrainPropertiesRef> NativeRef;
};
