// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_DelegateParticleData.generated.h"

USTRUCT(BlueprintType)
struct AGXUNREAL_API FDelegateParticleData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Particle Data")
	TArray<FVector4> PositionsAndRadii;

	UPROPERTY(BlueprintReadOnly, Category = "Particle Data")
	TArray<FVector4> VelocitiesAndMasses;

	UPROPERTY(BlueprintReadOnly, Category = "Particle Data")
	TArray<FVector4> Orientations;

	UPROPERTY(BlueprintReadOnly, Category = "Particle Data")
	TArray<bool> Exists;

	UPROPERTY(BlueprintReadOnly, Category = "Particle Data")
	int32 ParticleCount {0};
};
