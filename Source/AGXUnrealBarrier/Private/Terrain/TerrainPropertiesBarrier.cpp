// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/TerrainPropertiesBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/AGXTypeConversions.h"

FTerrainPropertiesBarrier::FTerrainPropertiesBarrier()
	: NativeRef(new FTerrainPropertiesRef)
{
}

FTerrainPropertiesBarrier::FTerrainPropertiesBarrier(std::shared_ptr<FTerrainPropertiesRef> Native)
	: NativeRef(std::move(Native))
{
	check(NativeRef);
}

void FTerrainPropertiesBarrier::SetCreateParticles(bool CreateParticles)
{
	check(HasNative());
	NativeRef->Native->setCreateParticles(CreateParticles);
}

bool FTerrainPropertiesBarrier::GetCreateParticles() const
{
	check(HasNative());
	return NativeRef->Native->getCreateParticles();
}

void FTerrainPropertiesBarrier::SetDeleteParticlesOutsideBounds(bool DeleteParticlesOutsideBounds)
{
	check(HasNative());
	NativeRef->Native->setDeleteSoilParticlesOutsideBounds(DeleteParticlesOutsideBounds);
}

bool FTerrainPropertiesBarrier::GetDeleteParticlesOutsideBounds() const
{
	check(HasNative());
	return NativeRef->Native->getDeleteSoilParticlesOutsideBounds();
}

void FTerrainPropertiesBarrier::SetPenetrationForceVelocityScaling(
	double PenetrationForceVelocityScaling)
{
	check(HasNative());
	NativeRef->Native->setPenetrationForceVelocityScaling(PenetrationForceVelocityScaling);
}

double FTerrainPropertiesBarrier::GetPenetrationForceVelocityScaling() const
{
	check(HasNative());
	return NativeRef->Native->getPenetrationForceVelocityScaling();
}

void FTerrainPropertiesBarrier::SetMaximumParticleActivationVolume(
	double MaximumParticleActivationVolume)
{
	check(HasNative());
	NativeRef->Native->setMaximumParticleActivationVolume(
		ConvertVolumeToAGX(MaximumParticleActivationVolume));
}

double FTerrainPropertiesBarrier::GetMaximumParticleActivationVolume() const
{
	check(HasNative());
	return ConvertVolumeToUnreal<double>(NativeRef->Native->getMaximumParticleActivationVolume());
}

void FTerrainPropertiesBarrier::SetSoilParticleSizeScaling(float Scaling)
{
	check(HasNative());
	NativeRef->Native->setSoilParticleSizeScaling(Scaling);
}

float FTerrainPropertiesBarrier::GetSoilParticleSizeScaling() const
{
	check(HasNative());
	return NativeRef->Native->getSoilParticleSizeScaling();
}

void FTerrainPropertiesBarrier::SetSoilParticleGrowthRate(double InRate)
{
	check(HasNative());
	NativeRef->Native->setSoilParticleGrowthRate(InRate);
}

double FTerrainPropertiesBarrier::GetSoilParticleGrowthRate() const
{
	check(HasNative());
	return NativeRef->Native->getSoilParticleGrowthRate();
}

void FTerrainPropertiesBarrier::SetSoilParticleMergeRate(double InRate)
{
	check(HasNative());
	NativeRef->Native->setSoilParticleMergeRate(InRate);
}

double FTerrainPropertiesBarrier::GetSoilParticleMergeRate() const
{
	check(HasNative());
	return NativeRef->Native->getSoilParticleMergeRate();
}

void FTerrainPropertiesBarrier::SetSoilParticleMergeThreshold(double InThreshold)
{
	check(HasNative());
	NativeRef->Native->setSoilParticleMergeThreshold(InThreshold);
}

double FTerrainPropertiesBarrier::GetSoilParticleMergeThreshold() const
{
	check(HasNative());
	return NativeRef->Native->getSoilParticleMergeThreshold();
}

void FTerrainPropertiesBarrier::SetSoilMergeSpeedThreshold(double InThreshold)
{
	check(HasNative());
	NativeRef->Native->setSoilMergeSpeedThreshold(ConvertDistanceToAGX(InThreshold));
}

double FTerrainPropertiesBarrier::GetSoilMergeSpeedThreshold() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeRef->Native->getSoilMergeSpeedThreshold());
}

void FTerrainPropertiesBarrier::SetSoilParticleLifeTime(double InLifeTime)
{
	check(HasNative());
	NativeRef->Native->setSoilParticleLifeTime(InLifeTime);
}

double FTerrainPropertiesBarrier::GetSoilParticleLifeTime() const
{
	check(HasNative());
	return NativeRef->Native->getSoilParticleLifeTime();
}

void FTerrainPropertiesBarrier::SetEnableAvalanching(bool Enable)
{
	check(HasNative());
	NativeRef->Native->setEnableAvalanching(Enable);
}

bool FTerrainPropertiesBarrier::GetEnableAvalanching() const
{
	check(HasNative());
	return NativeRef->Native->getEnableAvalanching();
}

void FTerrainPropertiesBarrier::SetAvalancheMaxHeightGrowth(double InValue)
{
	check(HasNative());
	NativeRef->Native->setAvalancheMaxHeightGrowth(ConvertDistanceToAGX(InValue));
}

double FTerrainPropertiesBarrier::GetAvalancheMaxHeightGrowth() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeRef->Native->getAvalancheMaxHeightGrowth());
}

void FTerrainPropertiesBarrier::SetAvalancheDecayFraction(double InValue)
{
	check(HasNative());
	NativeRef->Native->setAvalancheDecayFraction(InValue);
}

double FTerrainPropertiesBarrier::GetAvalancheDecayFraction() const
{
	check(HasNative());
	return NativeRef->Native->getAvalancheDecayFraction();
}

void FTerrainPropertiesBarrier::SetAvalancheErrorThreshold(double InValue)
{
	check(HasNative());
	NativeRef->Native->setAvalancheErrorThreshold(InValue);
}

double FTerrainPropertiesBarrier::GetAvalancheErrorThreshold() const
{
	check(HasNative());
	return NativeRef->Native->getAvalancheErrorThreshold();
}

void FTerrainPropertiesBarrier::SetActivationSpeed(double InSpeed)
{
	check(HasNative());
	NativeRef->Native->setActivationSpeed(ConvertDistanceToAGX(InSpeed));
}

double FTerrainPropertiesBarrier::GetActivationSpeed() const
{
	check(HasNative());
	return ConvertDistanceToUnreal<double>(NativeRef->Native->getActivationSpeed());
}

void FTerrainPropertiesBarrier::SetEnableDeformation(bool bEnable)
{
	check(HasNative());
	NativeRef->Native->setEnableDeformation(bEnable);
}

bool FTerrainPropertiesBarrier::GetEnableDeformation() const
{
	check(HasNative());
	return NativeRef->Native->getEnableDeformation();
}

void FTerrainPropertiesBarrier::SetEnableLockedBorders(bool Enable)
{
	check(HasNative());
	NativeRef->Native->setEnableLockedBorders(Enable);
}

bool FTerrainPropertiesBarrier::GetEnableLockedBorders() const
{
	check(HasNative());
	return NativeRef->Native->getEnableLockedBorders();
}

void FTerrainPropertiesBarrier::SetEnableSoilCompaction(bool Enable)
{
	check(HasNative());
	NativeRef->Native->setEnableSoilCompaction(Enable);
}

bool FTerrainPropertiesBarrier::GetEnableSoilCompaction() const
{
	check(HasNative());
	return NativeRef->Native->getEnableSoilCompaction();
}

bool FTerrainPropertiesBarrier::HasNative() const
{
	return NativeRef && NativeRef->Native != nullptr;
}

void FTerrainPropertiesBarrier::AllocateNative()
{
	check(!HasNative());
	NativeRef->Native = new agxTerrain::TerrainProperties();
}

FTerrainPropertiesRef* FTerrainPropertiesBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FTerrainPropertiesRef* FTerrainPropertiesBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

void FTerrainPropertiesBarrier::ReleaseNative()
{
	if (!HasNative())
		return;

	NativeRef->Native = nullptr;
}
