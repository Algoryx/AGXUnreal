// Copyright 2025, Algoryx Simulation AB.

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

	bool operator==(const FAGX_ShapeMaterialWireProperties& Other) const = default;

	/**
	 * Young's modulus when stretching the wire [Pa].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Material Wire Properties",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real YoungsModulusStretch;

	/**
	 * Spook Damping when stretching the wire [s].
	 * The value is the time the constraint has to fulfill its violation.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Material Wire Properties",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real SpookDampingStretch;

	/**
	 * Young's modulus when bending the wire [Pa].
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Material Wire Properties",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real YoungsModulusBend;

	/**
	 * Spook Damping when bending the wire [s].
	 * The value is the time the constraint has to fulfill its violation.
	 */
	UPROPERTY(
		EditAnywhere, Category = "AGX Material Wire Properties",
		Meta = (ClampMin = "0.0", UIMin = "0.0"))
	FAGX_Real SpookDampingBend;

public:
	FAGX_ShapeMaterialWireProperties();
};
