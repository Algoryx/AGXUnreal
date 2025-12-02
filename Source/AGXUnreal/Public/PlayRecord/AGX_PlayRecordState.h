// Copyright 2025, Algoryx Simulation AB.

#pragma once

// AGX Dynamics for Unreal includes.
#include "AGX_Real.h"

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_PlayRecordState.generated.h"

USTRUCT(BlueprintType)
struct AGXUNREAL_API FAGX_PlayRecordState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Play Record")
	TArray<FAGX_Real> Values;
};
