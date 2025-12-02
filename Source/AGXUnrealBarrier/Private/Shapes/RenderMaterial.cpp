// Copyright 2025, Algoryx Simulation AB.

#include "Shapes/RenderMaterial.h"

FLinearColor FAGX_RenderMaterial::ConvertToLinear(const FVector4& SRGBColor)
{
	// Convert from AGX Dynamics sRGB (float 0..1) to Unreal Engine sRGB (uint8, 0..255).
	const FColor SRGB(
		static_cast<uint8>(SRGBColor.X * 255.0f), static_cast<uint8>(SRGBColor.Y * 255.0f),
		static_cast<uint8>(SRGBColor.Z * 255.0f), static_cast<uint8>(SRGBColor.W * 255.0f));

	// Convert from Unreal Engine sRGB (uint8, 0..255) to linear color space (complicated).
	const FLinearColor Linear(SRGB);

	return Linear;
}
