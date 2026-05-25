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
	Normal
};

class AGXUNREAL_API FOpenPLX_RenderUtilities
{
public:
	static UTexture2D* CreateTexture(
		const FOpenPLXTextureData& TextureData, UObject& Owner, EOpenPLX_TextureUsage Usage);
};
