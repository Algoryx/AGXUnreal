// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "Misc/Optional.h"

// Standard library includes.
#include <memory>

#include "OpenPLXMaterialBarrier.generated.h"

struct FOpenPLXMaterialRef;

USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FOpenPLXTextureData
{
	GENERATED_BODY()

	FString Name; // Name of the Texture object.
	FGuid Guid; // Guid of the Texture object.
	FString TextureDataName; // Name of the underlying pixel data object.
	FGuid TextureDataGuid; // Guid of the underlying pixel data object.
	int32 Width {0};
	int32 Height {0};
	int32 NumChannels {0};
	FString Swizzle;
	TArray<uint8> Pixels;
};

USTRUCT(BlueprintType)
struct AGXUNREALBARRIER_API FOpenPLXMaterialBarrier
{
	GENERATED_BODY()

	FOpenPLXMaterialBarrier();
	FOpenPLXMaterialBarrier(std::shared_ptr<FOpenPLXMaterialRef> Native);

	bool HasNative() const;
	FOpenPLXMaterialRef* GetNative();
	const FOpenPLXMaterialRef* GetNative() const;

	FString GetName() const;
	FGuid GetGuid() const;
	bool HasTrait(const FString& Trait) const;
	TOptional<FLinearColor> GetBaseColor() const;
	TOptional<FOpenPLXTextureData> GetBaseColorTextureData() const;
	TOptional<float> GetMetallic() const;
	TOptional<FOpenPLXTextureData> GetMetallicTextureData() const;
	TOptional<float> GetRoughness() const;
	TOptional<FOpenPLXTextureData> GetRoughnessTextureData() const;
	TOptional<float> GetAlpha() const;
	TOptional<FOpenPLXTextureData> GetAlphaTextureData() const;
	TOptional<float> GetNormalScale() const;
	TOptional<FOpenPLXTextureData> GetNormalTextureData() const;
	TOptional<FOpenPLXTextureData> GetAmbientOcclusionTextureData() const;

private:
	std::shared_ptr<FOpenPLXMaterialRef> NativeRef;
};
