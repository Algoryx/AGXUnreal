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

void FTerrainWheelDeformationPropertiesBarrier::SetEnableTerrainDeformation(bool InEnable)
{
	check(HasNative());
	NativeRef->Native->setEnableDeformation(InEnable);
}

bool FTerrainWheelDeformationPropertiesBarrier::GetEnableTerrainDeformation() const
{
	check(HasNative());
	return NativeRef->Native->getEnableDeformation();
}

void FTerrainWheelDeformationPropertiesBarrier::SetEnableTerrainDisplacement(bool InEnable)
{
	check(HasNative());
	NativeRef->Native->setEnableDisplacement(InEnable);
}

bool FTerrainWheelDeformationPropertiesBarrier::GetEnableTerrainDisplacement() const
{
	check(HasNative());
	return NativeRef->Native->getEnableDisplacement();
}

void FTerrainWheelDeformationPropertiesBarrier::SetDisplacementModel(
	EAGX_TerrainWheelDisplacementModel InModel)
{
	check(HasNative());
	NativeRef->Native->setDisplacementModel(Convert(InModel));
}

EAGX_TerrainWheelDisplacementModel
FTerrainWheelDeformationPropertiesBarrier::GetDisplacementModel() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getDisplacementModel());
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

FGuid FTerrainWheelDeformationPropertiesBarrier::GetGuid() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getUuid());
}
