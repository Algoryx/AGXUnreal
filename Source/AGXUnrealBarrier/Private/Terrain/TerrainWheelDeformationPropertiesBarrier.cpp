// Copyright 2026, Algoryx Simulation AB.

#include "Terrain/TerrainWheelDeformationPropertiesBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/AGXTypeConversions.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include <agxTerrain/WheelDeformationProperties.h>
#include "EndAGXIncludes.h"

FTerrainWheelDeformationPropertiesBarrier::FTerrainWheelDeformationPropertiesBarrier()
	: NativeRef {new FTerrainWheelDeformationPropertiesRef}
{
}

FTerrainWheelDeformationPropertiesBarrier::FTerrainWheelDeformationPropertiesBarrier(
	std::shared_ptr<FTerrainWheelDeformationPropertiesRef> Native)
	: NativeRef {std::move(Native)}
{
}

bool FTerrainWheelDeformationPropertiesBarrier::HasNative() const
{
	return NativeRef->Native;
}

FTerrainWheelDeformationPropertiesRef* FTerrainWheelDeformationPropertiesBarrier::GetNative()
{
	return NativeRef.get();
}

const FTerrainWheelDeformationPropertiesRef* FTerrainWheelDeformationPropertiesBarrier::GetNative()
	const
{
	return NativeRef.get();
}

void FTerrainWheelDeformationPropertiesBarrier::AllocateNative()
{
	check(!HasNative());
	NativeRef->Native = new agxTerrain::WheelDeformationProperties();
}

void FTerrainWheelDeformationPropertiesBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->Native = nullptr;
}
