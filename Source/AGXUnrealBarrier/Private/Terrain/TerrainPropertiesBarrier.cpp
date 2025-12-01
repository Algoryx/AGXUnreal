// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/TerrainPropertiesBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"
#include "TypeConversions.h"

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
	NativeRef->Native->setDeleteSoilParticlesOutsideBounds(
		DeleteParticlesOutsideBounds);
}

bool FTerrainPropertiesBarrier::GetDeleteParticlesOutsideBounds() const
{
	check(HasNative());
	return NativeRef->Native->getDeleteSoilParticlesOutsideBounds();
}

void FTerrainPropertiesBarrier::SetPenetrationForceVelocityScaling(double PenetrationForceVelocityScaling)
{
	check(HasNative());
	NativeRef->Native->setPenetrationForceVelocityScaling(
		PenetrationForceVelocityScaling);
}

double FTerrainPropertiesBarrier::GetPenetrationForceVelocityScaling() const
{
	check(HasNative());
	return NativeRef->Native->getPenetrationForceVelocityScaling();
}

void FTerrainPropertiesBarrier::SetMaximumParticleActivationVolume(double MaximumParticleActivationVolume)
{
	check(HasNative());
	NativeRef->Native->setMaximumParticleActivationVolume(
		ConvertVolumeToAGX(MaximumParticleActivationVolume));
}

double FTerrainPropertiesBarrier::GetMaximumParticleActivationVolume() const
{
	check(HasNative());
	return ConvertVolumeToUnreal<double>(
		NativeRef->Native->getMaximumParticleActivationVolume());
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
