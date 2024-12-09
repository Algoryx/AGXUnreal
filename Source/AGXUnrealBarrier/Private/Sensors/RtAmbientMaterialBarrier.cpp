// Copyright 2024, Algoryx Simulation AB.

#include "Sensors/RtAmbientMaterialBarrier.h"

// AGX Dynamics for Unreal includes.
#include "AGX_Check.h"
#include "Sensors/SensorRef.h"

FRtAmbientMaterialBarrier::FRtAmbientMaterialBarrier()
	: Native(std::make_shared<FRtAmbientMaterial>())
{
}

FRtAmbientMaterialBarrier::~FRtAmbientMaterialBarrier()
{
}

void FRtAmbientMaterialBarrier::AllocateNative()
{
	check(!HasNative());
	Native->Native = std::make_shared<agxSensor::RtAmbientMaterial>();
	*Native->Native = agxSensor::RtAmbientMaterial::create();
}

bool FRtAmbientMaterialBarrier::HasNative() const
{
	AGX_CHECK(Native != nullptr);
	return Native->Native != nullptr && Native->Native->isValid();
}

FRtAmbientMaterial* FRtAmbientMaterialBarrier::GetNative()
{
	check(HasNative());
	return Native.get();
}

const FRtAmbientMaterial* FRtAmbientMaterialBarrier::GetNative() const
{
	check(HasNative());
	return Native.get();
}

void FRtAmbientMaterialBarrier::ReleaseNative()
{
	Native->Native = nullptr;
}

void FRtAmbientMaterialBarrier::SetRefractiveIndex(float InRefractiveIndex)
{
	check(HasNative());
	Native->Native->setRefractiveIndex(InRefractiveIndex);
}

float FRtAmbientMaterialBarrier::GetRefractiveIndex() const
{
	check(HasNative());
	return Native->Native->getRefractiveIndex();
}

void FRtAmbientMaterialBarrier::SetAttenuationCoefficient(float InAttenuationCoefficient)
{
	check(HasNative());
	Native->Native->setAttenuationCoefficient(InAttenuationCoefficient);
}

float FRtAmbientMaterialBarrier::GetAttenuationCoefficient() const
{
	check(HasNative());
	return Native->Native->getAttenuationCoefficient();
}

void FRtAmbientMaterialBarrier::SetReturnProbabilityScaling(float InScalingParameter)
{
	check(HasNative());
	Native->Native->setReturnProbabilityScaling(InScalingParameter);
}

float FRtAmbientMaterialBarrier::GetReturnProbabilityScaling() const
{
	check(HasNative());
	return Native->Native->getReturnProbabilityScaling();
}

void FRtAmbientMaterialBarrier::SetReturnGammaDistributionShapeParameter(float InShapeParameter)
{
	check(HasNative());
	Native->Native->setReturnGammaDistributionShapeParameter(InShapeParameter);
}

float FRtAmbientMaterialBarrier::GetReturnGammaDistributionShapeParameter() const
{
	check(HasNative());
	return Native->Native->getReturnGammaDistributionShapeParameter();
}

void FRtAmbientMaterialBarrier::SetReturnGammaDistributionScaleParameter(float InScaleParameter)
{
	check(HasNative());
	Native->Native->setReturnGammaDistributionScaleParameter(InScaleParameter);
}

float FRtAmbientMaterialBarrier::GetReturnGammaDistributionScaleParameter() const
{
	check(HasNative());
	return Native->Native->getReturnGammaDistributionScaleParameter();
}

void FRtAmbientMaterialBarrier::ConfigureAsAir(float Visibility)
{
	check(HasNative());
	Native->Native->configureAsAir(Visibility);
}

void FRtAmbientMaterialBarrier::ConfigureAsFog(
	float Visibility, float Wavelength, float Maritimeness)
{
	check(HasNative());
	Native->Native->configureAsFog(Visibility, Wavelength, Maritimeness);
}

void FRtAmbientMaterialBarrier::ConfigureAsRainfall(float Rate, float Tropicalness)
{
	check(HasNative());
	Native->Native->configureAsRainfall(Rate, Tropicalness);
}

void FRtAmbientMaterialBarrier::ConfigureAsSnowfall(float Rate, float Wavelength)
{
	check(HasNative());
	Native->Native->configureAsSnowfall(Rate, Wavelength);
}
