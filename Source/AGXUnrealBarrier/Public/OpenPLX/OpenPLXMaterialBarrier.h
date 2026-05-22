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

	FString Name;
	FGuid Guid;
	int32 Width {0};
	int32 Height {0};
	int32 NumChannels {0};
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

private:
	std::shared_ptr<FOpenPLXMaterialRef> NativeRef;
};
