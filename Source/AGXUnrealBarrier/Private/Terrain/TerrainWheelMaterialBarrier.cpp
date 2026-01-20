// Copyright 2025, Algoryx Simulation AB.

#include "Terrain/TerrainWheelMaterialBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "BarrierOnly/AGXRefs.h"

FTerrainWheelMaterialBarrier::FTerrainWheelMaterialBarrier()
	: NativePtr(std::make_shared<FTerrainWheelMaterialPtr>())
{
}

FTerrainWheelMaterialBarrier::FTerrainWheelMaterialBarrier(
	std::shared_ptr<FTerrainWheelMaterialPtr> InNativeRef)
	: NativePtr {std::move(InNativeRef)}
{
}

void FTerrainWheelMaterialBarrier::AllocateNative()
{
	check(!HasNative());
	NativePtr->Native = std::make_shared<agxTerrain::TerrainWheelTerrainMaterialParameters>();
}

void FTerrainWheelMaterialBarrier::SetSinkageExponentParameterA(double Value)
{
	check(HasNative());
	NativePtr->Native->sinkageExponentParameterA = Value;
}

double FTerrainWheelMaterialBarrier::GetSinkageExponentParameterA() const
{
	check(HasNative());
	return NativePtr->Native->sinkageExponentParameterA;
}

void FTerrainWheelMaterialBarrier::SetSinkageExponentParameterB(double Value)
{
	check(HasNative());
	NativePtr->Native->sinkageExponentParameterB = Value;
}

double FTerrainWheelMaterialBarrier::GetSinkageExponentParameterB() const
{
	check(HasNative());
	return NativePtr->Native->sinkageExponentParameterB;
}

void FTerrainWheelMaterialBarrier::SetCohesion(double Value)
{
	check(HasNative());
	NativePtr->Native->cohesion = Value;
}

double FTerrainWheelMaterialBarrier::GetCohesion() const
{
	check(HasNative());
	return NativePtr->Native->cohesion;
}

void FTerrainWheelMaterialBarrier::SetAngleOfInternalFriction(double Value)
{
	check(HasNative());
	NativePtr->Native->angleOfInternalFriction = Value;
}

double FTerrainWheelMaterialBarrier::GetAngleOfInternalFriction() const
{
	check(HasNative());
	return NativePtr->Native->angleOfInternalFriction;
}

void FTerrainWheelMaterialBarrier::SetShearModulusXParameterA(double Value)
{
	check(HasNative());
	NativePtr->Native->shearModulusXParameterA = Value;
}

double FTerrainWheelMaterialBarrier::GetShearModulusXParameterA() const
{
	check(HasNative());
	return NativePtr->Native->shearModulusXParameterA;
}

void FTerrainWheelMaterialBarrier::SetShearModulusXParameterB(double Value)
{
	check(HasNative());
	NativePtr->Native->shearModulusXParameterB = Value;
}

double FTerrainWheelMaterialBarrier::GetShearModulusXParameterB() const
{
	check(HasNative());
	return NativePtr->Native->shearModulusXParameterB;
}

void FTerrainWheelMaterialBarrier::SetShearModulusYParameterA(double Value)
{
	check(HasNative());
	NativePtr->Native->shearModulusYParameterA = Value;
}

double FTerrainWheelMaterialBarrier::GetShearModulusYParameterA() const
{
	check(HasNative());
	return NativePtr->Native->shearModulusYParameterA;
}

void FTerrainWheelMaterialBarrier::SetShearModulusYParameterB(double Value)
{
	check(HasNative());
	NativePtr->Native->shearModulusYParameterB = Value;
}

double FTerrainWheelMaterialBarrier::GetShearModulusYParameterB() const
{
	check(HasNative());
	return NativePtr->Native->shearModulusYParameterB;
}

void FTerrainWheelMaterialBarrier::SetCohesiveModulusBekker(double Value)
{
	check(HasNative());
	NativePtr->Native->cohesiveModulusBekker = Value;
}

double FTerrainWheelMaterialBarrier::GetCohesiveModulusBekker() const
{
	check(HasNative());
	return NativePtr->Native->cohesiveModulusBekker;
}

void FTerrainWheelMaterialBarrier::SetFrictionalModulusBekker(double Value)
{
	check(HasNative());
	NativePtr->Native->frictionalModulusBekker = Value;
}

double FTerrainWheelMaterialBarrier::GetFrictionalModulusBekker() const
{
	check(HasNative());
	return NativePtr->Native->frictionalModulusBekker;
}

void FTerrainWheelMaterialBarrier::SetCohesiveModulusReece(double Value)
{
	check(HasNative());
	NativePtr->Native->cohesiveModulusReece = Value;
}

double FTerrainWheelMaterialBarrier::GetCohesiveModulusReece() const
{
	check(HasNative());
	return NativePtr->Native->cohesiveModulusReece;
}

void FTerrainWheelMaterialBarrier::SetFrictionalModulusReece(double Value)
{
	check(HasNative());
	NativePtr->Native->frictionalModulusReece = Value;
}

double FTerrainWheelMaterialBarrier::GetFrictionalModulusReece() const
{
	check(HasNative());
	return NativePtr->Native->frictionalModulusReece;
}

void FTerrainWheelMaterialBarrier::SetMassDensity(double Value)
{
	check(HasNative());
	NativePtr->Native->massDensity = Value;
}

double FTerrainWheelMaterialBarrier::GetMassDensity() const
{
	check(HasNative());
	return NativePtr->Native->massDensity;
}

void FTerrainWheelMaterialBarrier::SetMaximumNormalStressAngleParameterA(double Value)
{
	check(HasNative());
	NativePtr->Native->maximumNormalStressAngleParameterA = Value;
}

double FTerrainWheelMaterialBarrier::GetMaximumNormalStressAngleParameterA() const
{
	check(HasNative());
	return NativePtr->Native->maximumNormalStressAngleParameterA;
}

void FTerrainWheelMaterialBarrier::SetMaximumNormalStressAngleParameterB(double Value)
{
	check(HasNative());
	NativePtr->Native->maximumNormalStressAngleParameterB = Value;
}

double FTerrainWheelMaterialBarrier::GetMaximumNormalStressAngleParameterB() const
{
	check(HasNative());
	return NativePtr->Native->maximumNormalStressAngleParameterB;
}

void FTerrainWheelMaterialBarrier::SetRearAngleParameterA(double Value)
{
	check(HasNative());
	NativePtr->Native->rearAngleParameterA = Value;
}

double FTerrainWheelMaterialBarrier::GetRearAngleParameterA() const
{
	check(HasNative());
	return NativePtr->Native->rearAngleParameterA;
}

void FTerrainWheelMaterialBarrier::SetRearAngleParameterB(double Value)
{
	check(HasNative());
	NativePtr->Native->rearAngleParameterB = Value;
}

double FTerrainWheelMaterialBarrier::GetRearAngleParameterB() const
{
	check(HasNative());
	return NativePtr->Native->rearAngleParameterB;
}

bool FTerrainWheelMaterialBarrier::HasNative() const
{
	AGX_CHECK(NativePtr != nullptr);
	return NativePtr->Native != nullptr;
}

FTerrainWheelMaterialPtr* FTerrainWheelMaterialBarrier::GetNative()
{
	check(HasNative());
	return NativePtr.get();
}

const FTerrainWheelMaterialPtr* FTerrainWheelMaterialBarrier::GetNative() const
{
	check(HasNative());
	return NativePtr.get();
}

void FTerrainWheelMaterialBarrier::ReleaseNative()
{
	NativePtr->Native = nullptr;
}
