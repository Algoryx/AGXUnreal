// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/TerrainWheelBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/AGXTypeConversions.h"


FTerrainWheelBarrier::FTerrainWheelBarrier()
	: NativeRef {new FTerrainWheelRef}
{
}

FTerrainWheelBarrier::FTerrainWheelBarrier(std::shared_ptr<FTerrainWheelRef> InNativeRef)
	: NativeRef {std::move(InNativeRef)}
{
}

void FTerrainWheelBarrier::AllocateNative(double Radius, double Width)
{
	check(!HasNative());
	const double RadiusAGX = ConvertDistanceToAGX(Radius);
	const double WidthAGX = ConvertDistanceToAGX(Width);
	NativeRef->Native = new agxTerrain::TerrainWheel(RadiusAGX, WidthAGX);
}

FGuid FTerrainWheelBarrier::GetGuid() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getUuid());
}

bool FTerrainWheelBarrier::HasNative() const
{
	return NativeRef->Native != nullptr;
}

FTerrainWheelRef* FTerrainWheelBarrier::GetNative()
{
	check(HasNative());
	return NativeRef.get();
}

const FTerrainWheelRef* FTerrainWheelBarrier::GetNative() const
{
	check(HasNative());
	return NativeRef.get();
}

uint64 FTerrainWheelBarrier::GetNativeAddress() const
{
	return HasNative() ? reinterpret_cast<uint64>(NativeRef->Native.get()) : 0;
}

void FTerrainWheelBarrier::SetNativeAddress(uint64 Address)
{
	NativeRef->Native = reinterpret_cast<agxTerrain::TerrainWheel*>(Address);
}

void FTerrainWheelBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->Native = nullptr;
}

void FTerrainWheelBarrier::IncrementRefCount() const
{
	check(HasNative());
	NativeRef->Native->reference();
}

void FTerrainWheelBarrier::DecrementRefCount() const
{
	check(HasNative());
	NativeRef->Native->unreference();
}
