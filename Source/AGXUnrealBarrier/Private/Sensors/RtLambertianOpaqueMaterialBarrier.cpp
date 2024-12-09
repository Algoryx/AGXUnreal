// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/RtLambertianOpaqueMaterialBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "Sensors/SensorRef.h"

FRtLambertianOpaqueMaterialBarrier::FRtLambertianOpaqueMaterialBarrier()
	: Native(std::make_shared<FRtLambertianOpaqueMaterial>())
{
}

FRtLambertianOpaqueMaterialBarrier::~FRtLambertianOpaqueMaterialBarrier()
{
}

void FRtLambertianOpaqueMaterialBarrier::SetReflectivity(float Reflectivity)
{
	check(HasNative());
	Native->Native->setReflectivity(Reflectivity);
}

float FRtLambertianOpaqueMaterialBarrier::GetReflectivity() const
{
	check(HasNative());
	return Native->Native->getReflectivity();
}

void FRtLambertianOpaqueMaterialBarrier::AllocateNative()
{
	check(!HasNative());
	Native->Native = std::make_shared<agxSensor::RtLambertianOpaqueMaterial>();
	*Native->Native = agxSensor::RtLambertianOpaqueMaterial::create();
}

bool FRtLambertianOpaqueMaterialBarrier::HasNative() const
{
	AGX_CHECK(Native != nullptr);
	return Native != nullptr && Native->Native != nullptr && Native->Native->isValid();
}

FRtLambertianOpaqueMaterial* FRtLambertianOpaqueMaterialBarrier::GetNative()
{
	check(HasNative());
	return Native.get();
}

const FRtLambertianOpaqueMaterial* FRtLambertianOpaqueMaterialBarrier::GetNative() const
{
	check(HasNative());
	return Native.get();
}

void FRtLambertianOpaqueMaterialBarrier::ReleaseNative()
{
	Native->Native = nullptr;
}
