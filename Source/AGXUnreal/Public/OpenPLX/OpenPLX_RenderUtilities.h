// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

class UTexture2D;
struct FOpenPLXTextureData;

enum class EOpenPLX_TextureUsage : uint8
{
	BaseColor,
	Scalar,
	Normal,
	Raw
};

class AGXUNREAL_API FOpenPLX_RenderUtilities
{
public:
	/**
	 * Creates a Texture from OpenPLX texture data.
	 * If bCreateRenderResource is false, the texture is initialized with CPU-side texture data only
	 * and UpdateResource is not called.
	 */
	static UTexture2D* CreateTexture(
		const FOpenPLXTextureData& TextureData, UObject& Owner, EOpenPLX_TextureUsage Usage,
		bool bCreateRenderResource = true);
};
