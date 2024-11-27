// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

// Standard library includes.
#include <memory>

struct FRtAmbientMaterial;

class AGXUNREALBARRIER_API FRtAmbientMaterialBarrier
{
public:
	FRtAmbientMaterialBarrier();
	virtual ~FRtAmbientMaterialBarrier();

	virtual void AllocateNative();

	bool HasNative() const;
	FRtAmbientMaterial* GetNative();
	const FRtAmbientMaterial* GetNative() const;
	void ReleaseNative();

	void SetRefractiveIndex(float InRefractiveIndex);
	float GetRefractiveIndex() const;

	void SetAttenuationCoefficient(float InAttenuationCoefficient);
	float GetAttenuationCoefficient() const;

	void SetReturnProbabilityScaling(float InScalingParameter);
	float GetReturnProbabilityScaling() const;

	void SetReturnGammaDistributionShapeParameter(float InShapeParameter);
	float GetReturnGammaDistributionShapeParameter() const;

	void SetReturnGammaDistributionScaleParameter(float InScaleParameter);
	float GetReturnGammaDistributionScaleParameter() const;

	/**
	 * Configure the material parameters as clear weather air with the specified visibility. This
	 * configuration uses fog-like atmospheric returns combined with (inverse) MOR attenuation.
	 * Visibility - visibility in kilometers
	 */
	void ConfigureAsAir(float Visibility);

	/**
	 * Configure the material parameters as foggy weather with the specified visibility. This
	 * configuration uses fog-like atmospheric returns combined with Al-Naboulsi attenuation.
	 * Visibility - visibility in kilometers
	 * Wavelength - sensor wavelength in nanometers
	 * Maritimeness - interpolation value between continental (0.0) and maritime (1.0) fog
	 */
	void ConfigureAsFog(float Visibility, float Wavelength, float Maritimeness = 0.0f);

	/**
	 * Configure the material parameters as rainfall with the specified precipitation rate. This
	 * configuration uses rain-like atmospheric returns combined with Carbonneau rainfall
	 * attenuation.
	 * Rate - precipitation rate in mm/h
	 * Tropicalness - interpolation value between light (0.0) and tropical (1.0) rain
	 */
	void ConfigureAsRainfall(float Rate, float Tropicalness = 0.0f);

	/**
	 * Configure the material parameters as light snowfall with the specified precipitation rate.
	 * This configuration uses rain-like atmospheric return combined with Carbonneau snowfall
	 * attenuation.
	 * Rate - precipitation rate in mm/h
	 * Wavelength - sensor wavelength in nanometers
	 */
	void ConfigureAsSnowfall(float Rate, float Wavelength);


protected:
	std::shared_ptr<FRtAmbientMaterial> Native;
};
