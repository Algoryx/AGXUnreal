// Copyright 2025, Algoryx Simulation AB.

#include "Materials/ShapeMaterialBarrier.h"

// AGX Dynamics for Unreal includes.
#include "BarrierOnly/AGXRefs.h"
#include "TypeConversions.h"

// AGX Dynamics includes.
#include <Misc/AssertionMacros.h>

FShapeMaterialBarrier::FShapeMaterialBarrier()
	: NativeRef {new FMaterialRef}
{
}

FShapeMaterialBarrier::FShapeMaterialBarrier(FShapeMaterialBarrier&& Other)
	: NativeRef {std::move(Other.NativeRef)}
{
}

FShapeMaterialBarrier::FShapeMaterialBarrier(std::unique_ptr<FMaterialRef> Native)
	: NativeRef(std::move(Native))
{
}

FShapeMaterialBarrier::~FShapeMaterialBarrier()
{
	// Must provide a destructor implementation in the .cpp file because the
	// std::unique_ptr NativeRef's destructor must be able to see the definition,
	// not just the forward declaration, of FMaterialRef.
}

bool FShapeMaterialBarrier::HasNative() const
{
	return NativeRef && NativeRef->Native;
}

FMaterialRef* FShapeMaterialBarrier::GetNative()
{
	return NativeRef.get();
}

const FMaterialRef* FShapeMaterialBarrier::GetNative() const
{
	return NativeRef.get();
}

void FShapeMaterialBarrier::AllocateNative(const FString& Name)
{
	check(!HasNative());
	NativeRef->Native = new agx::Material(Convert(Name));
}

void FShapeMaterialBarrier::ReleaseNative()
{
	check(HasNative());
	NativeRef->Native = nullptr;
}

void FShapeMaterialBarrier::SetName(const FString& Name)
{
	check(HasNative());
	NativeRef->Native->setName(Convert(Name));
}

FString FShapeMaterialBarrier::GetName() const
{
	check(HasNative());
	return Convert(NativeRef->Native->getName());
}

void FShapeMaterialBarrier::SetDensity(double Density)
{
	check(HasNative());
	NativeRef->Native->getBulkMaterial()->setDensity(Density);
}

double FShapeMaterialBarrier::GetDensity() const
{
	check(HasNative());
	return NativeRef->Native->getBulkMaterial()->getDensity();
}

void FShapeMaterialBarrier::SetYoungsModulus(double YoungsModulus)
{
	check(HasNative());
	NativeRef->Native->getBulkMaterial()->setYoungsModulus(YoungsModulus);
}

double FShapeMaterialBarrier::GetYoungsModulus() const
{
	check(HasNative());
	return NativeRef->Native->getBulkMaterial()->getYoungsModulus();
}

void FShapeMaterialBarrier::SetBulkViscosity(double Viscosity)
{
	check(HasNative());
	NativeRef->Native->getBulkMaterial()->setViscosity(Viscosity);
}

double FShapeMaterialBarrier::GetBulkViscosity() const
{
	check(HasNative());
	return NativeRef->Native->getBulkMaterial()->getViscosity();
}

void FShapeMaterialBarrier::SetSpookDamping(double SpookDamping)
{
	check(HasNative());
	NativeRef->Native->getBulkMaterial()->setDamping(SpookDamping);
}

double FShapeMaterialBarrier::GetSpookDamping() const
{
	check(HasNative());
	return NativeRef->Native->getBulkMaterial()->getDamping();
}

void FShapeMaterialBarrier::SetMinMaxElasticRestLength(
	double MinElasticRestLength, double MaxElasticRestLength)
{
	check(HasNative());

	NativeRef->Native->getBulkMaterial()->setMinMaxElasticRestLength(
		ConvertDistanceToAGX<agx::Real>(MinElasticRestLength),
		ConvertDistanceToAGX<agx::Real>(MaxElasticRestLength));
}

double FShapeMaterialBarrier::GetMinElasticRestLength() const
{
	check(HasNative());

	return ConvertDistanceToUnreal<double>(
		NativeRef->Native->getBulkMaterial()->getMinElasticRestLength());
}

double FShapeMaterialBarrier::GetMaxElasticRestLength() const
{
	check(HasNative());

	return ConvertDistanceToUnreal<double>(
		NativeRef->Native->getBulkMaterial()->getMaxElasticRestLength());
}

void FShapeMaterialBarrier::SetFrictionEnabled(bool bEnabled)
{
	check(HasNative());
	NativeRef->Native->getSurfaceMaterial()->setFrictionEnabled(bEnabled);
}

bool FShapeMaterialBarrier::GetFrictionEnabled() const
{
	check(HasNative());
	return NativeRef->Native->getSurfaceMaterial()->getFrictionEnabled();
}

void FShapeMaterialBarrier::SetRoughness(double Roughness)
{
	check(HasNative());
	NativeRef->Native->getSurfaceMaterial()->setRoughness(Roughness);
}

double FShapeMaterialBarrier::GetRoughness() const
{
	check(HasNative());
	return NativeRef->Native->getSurfaceMaterial()->getRoughness();
}

void FShapeMaterialBarrier::SetSurfaceViscosity(double Viscosity)
{
	check(HasNative());
	NativeRef->Native->getSurfaceMaterial()->setViscosity(Viscosity);
}

double FShapeMaterialBarrier::GetSurfaceViscosity() const
{
	check(HasNative());
	return NativeRef->Native->getSurfaceMaterial()->getViscosity();
}

void FShapeMaterialBarrier::SetAdhesion(double AdhesiveForce, double AdhesiveOverlap)
{
	check(HasNative());
	NativeRef->Native->getSurfaceMaterial()->setAdhesion(
		AdhesiveForce, ConvertDistanceToAGX<agx::Real>(AdhesiveOverlap));
}

double FShapeMaterialBarrier::GetAdhesiveForce() const
{
	check(HasNative());
	return NativeRef->Native->getSurfaceMaterial()->getAdhesion();
}

double FShapeMaterialBarrier::GetAdhesiveOverlap() const
{
	check(HasNative());

	return ConvertDistanceToUnreal<double>(
		NativeRef->Native->getSurfaceMaterial()->getAdhesiveOverlap());
}

// Wire properties.

double FShapeMaterialBarrier::GetYoungsModulusStretch() const
{
	check(HasNative());
	return NativeRef->Native->getWireMaterial()->getYoungsModulusStretch();
}

void FShapeMaterialBarrier::SetYoungsModulusStretch(double YoungsModulus) const
{
	check(HasNative());
	NativeRef->Native->getWireMaterial()->setYoungsModulusStretch(YoungsModulus);
}

double FShapeMaterialBarrier::GetYoungsModulusBend() const
{
	check(HasNative());
	return NativeRef->Native->getWireMaterial()->getYoungsModulusBend();
}

void FShapeMaterialBarrier::SetYoungsModulusBend(double YoungsModulus) const
{
	check(HasNative());
	NativeRef->Native->getWireMaterial()->setYoungsModulusBend(YoungsModulus);
}

double FShapeMaterialBarrier::GetSpookDampingStretch() const
{
	check(HasNative());
	return NativeRef->Native->getWireMaterial()->getDampingStretch();
}

void FShapeMaterialBarrier::SetSpookDampingStretch(double SpookDamping) const
{
	check(HasNative());
	NativeRef->Native->getWireMaterial()->setDampingStretch(SpookDamping);
}

double FShapeMaterialBarrier::GetSpookDampingBend() const
{
	check(HasNative());
	return NativeRef->Native->getWireMaterial()->getDampingBend();
}

void FShapeMaterialBarrier::SetSpookDampingBend(double SpookDamping) const
{
	check(HasNative());
	NativeRef->Native->getWireMaterial()->setDampingBend(SpookDamping);
}

FGuid FShapeMaterialBarrier::GetGuid() const
{
	check(HasNative());
	FGuid Guid = Convert(NativeRef->Native->getUuid());
	return Guid;
}
