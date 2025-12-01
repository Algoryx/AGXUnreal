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
