// Copyright 2026, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include <AGX_Real.h>

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ShapeMaterialWireProperties.generated.h"

USTRUCT()
struct AGXUNREAL_API FAGX_ShapeMaterialWireProperties
{
	GENERATED_BODY()

public:
	bool operator==(const FAGX_ShapeMaterialWireProperties& Other) const;

	/**
	 * Young's modulus when stretching the wire [Pa].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Material Wire Properties",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real YoungsModulusStretch;

	/**
	 * Attenuation when stretching the wire [steps].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Material Wire Properties",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real AttenuationStretch;

	/**
	 * Young's modulus when bending the wire [Pa].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Material Wire Properties",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real YoungsModulusBend;

	/**
	 * Attenuation when bending the wire [steps].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Material Wire Properties",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real AttenuationBend;

public:
	FAGX_ShapeMaterialWireProperties();
	void Serialize(FArchive& Archive);

private:
	UPROPERTY()
	FAGX_Real SpookDampingStretch_DEPRECATED;

	UPROPERTY()
	FAGX_Real SpookDampingBend_DEPRECATED;
};
