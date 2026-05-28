// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/TerrainWheelSettingsBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxTerrain/TerrainWheelSettings.h>
#include "EndAGXIncludes.h"

FTerrainWheelSettingsBarrier::FTerrainWheelSettingsBarrier()
	: NativeRef {new FTerrainWheelSettingsRef}
{
}

FTerrainWheelSettingsBarrier::FTerrainWheelSettingsBarrier(
	std::shared_ptr<FTerrainWheelSettingsRef> Native)
	: NativeRef {std::move(Native)}
{
}

bool FTerrainWheelSettingsBarrier::HasNative() const
{
	return NativeRef && NativeRef->Native;
}

FTerrainWheelSettingsRef* FTerrainWheelSettingsBarrier::GetNative()
{
	return NativeRef.get();
}

const FTerrainWheelSettingsRef* FTerrainWheelSettingsBarrier::GetNative() const
{
	return NativeRef.get();
}

void FTerrainWheelSettingsBarrier::AllocateNative()
{
	check(!HasNative());
	NativeRef->Native = new agxTerrain::TerrainWheelSettings();
}

void FTerrainWheelSettingsBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->Native = nullptr;
}
