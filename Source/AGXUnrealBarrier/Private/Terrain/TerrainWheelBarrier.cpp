// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/TerrainWheelBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "BarrierOnly/AGXRefs.h"
#include "BarrierOnly/AGXTypeConversions.h"
#include "RigidBodyBarrier.h"
#include "Shapes/CylinderShapeBarrier.h"

// AGX Dynamics includes.
#include "BeginAGXIncludes.h"
#include "agxCollide/Cylinder.h"
#include "EndAGXIncludes.h"

FTerrainWheelBarrier::FTerrainWheelBarrier()
	: NativeRef {new FTerrainWheelRef}
{
}

FTerrainWheelBarrier::FTerrainWheelBarrier(std::shared_ptr<FTerrainWheelRef> InNativeRef)
	: NativeRef {std::move(InNativeRef)}
{
}

void FTerrainWheelBarrier::SetEnableTerrainDeformation(bool InEnable)
{
	check(HasNative());
	NativeRef->Native->setOption(agxTerrain::TerrainWheel::ModelOptions::DEFORMATION, InEnable);
}

bool FTerrainWheelBarrier::GetEnableTerrainDeformation() const
{
	check(HasNative());
	return NativeRef->Native->getOption(agxTerrain::TerrainWheel::ModelOptions::DEFORMATION);
}

void FTerrainWheelBarrier::SetEnableTerrainDisplacement(bool InEnable)
{
	check(HasNative());
	NativeRef->Native->setOption(agxTerrain::TerrainWheel::ModelOptions::DISPLACEMENT, InEnable);
}

bool FTerrainWheelBarrier::GetEnableTerrainDisplacement() const
{
	check(HasNative());
	return NativeRef->Native->getOption(agxTerrain::TerrainWheel::ModelOptions::DISPLACEMENT);
}

void FTerrainWheelBarrier::SetEnableAGXDebugRendering(bool InEnable)
{
	check(HasNative());
	NativeRef->Native->setEnableDebugRegressionPlanes(InEnable);
}

void FTerrainWheelBarrier::AllocateNative(FRigidBodyBarrier& Body, FCylinderShapeBarrier& Cylinder)
{
	check(!HasNative());
	check(Body.HasNative());
	check(Cylinder.HasNative());
	agx::RigidBody* BodyAGX = Body.GetNative()->Native;
	agxCollide::Cylinder* CylinderAGX =
		dynamic_cast<agxCollide::Cylinder*>(Cylinder.GetNative()->NativeShape.get());
	AGX_CHECK(BodyAGX != nullptr);
	AGX_CHECK(CylinderAGX != nullptr);
	NativeRef->Native = new agxTerrain::TerrainWheel(BodyAGX, CylinderAGX);

	// TODO: this should not be needed (should be handled in AGX).
	auto Mat = Cylinder.GetNative()->NativeGeometry->getMaterial();
	NativeRef->Native->setMaterial(Mat);
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
