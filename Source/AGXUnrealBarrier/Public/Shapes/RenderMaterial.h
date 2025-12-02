// Copyright 2025, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Math/Vector.h"
#include "Misc/Guid.h"

#include "RenderMaterial.generated.h"

/**
 * A set of parameters compatible with the M_ImportedBase Render Material shipped with AGX Dynamics
 * for Unreal. Used during importing of e.g. .agx files to convert Render Data Render Materials to
 * Unreal Engine Render Materials.
 *
 * The data members may only be read if the corresponding bHas-bit has been set.
 *
 * The colors are in sRGB 0..1 format. To be used in a material they must be converted to the linear
 * color space.
 */
USTRUCT()
struct AGXUNREALBARRIER_API FAGX_RenderMaterial
{
	GENERATED_BODY()

	FAGX_RenderMaterial()
		: bHasAmbient(0)
		, bHasDiffuse(0)
		, bHasEmissive(0)
		, bHasShininess(0)
	{
	}

	// Bit-fields cannot have in-class/inline initializer so set in constructor instead.

	UPROPERTY()
	uint8 bHasAmbient : 1;

	UPROPERTY()
	uint8 bHasDiffuse : 1;

	UPROPERTY()
	uint8 bHasEmissive : 1;

	UPROPERTY()
	uint8 bHasShininess : 1;

	// Initializing a FVector4 from FVector::ZeroVector gives us black with full opacity.

	UPROPERTY()
	FVector4 Ambient = FVector::ZeroVector;

	UPROPERTY()
	FVector4 Diffuse = FVector::ZeroVector;

	UPROPERTY()
	FVector4 Emissive = FVector::ZeroVector;

	UPROPERTY()
	float Shininess = 0.0f;

	UPROPERTY()
	FName Name;

	FGuid Guid;

	/**
	 * Convert an sRGB 0..1 color, for example those stored in FAGX_RenderMaterial, to FLinearColor
	 * for use in Unreal Engine Render Material parameters.
	 *
	 * @return The given color in the linear color space.
	 */
	static FLinearColor ConvertToLinear(const FVector4& SRGBColor);
};
