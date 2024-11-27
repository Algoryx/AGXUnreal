// Copyright 2024, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Class.h"

#include "AGX_CustomPatternInterval.generated.h"

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_CustomPatternInterval
{
	GENERATED_BODY()

	FAGX_CustomPatternInterval() = default;
	FAGX_CustomPatternInterval(int32 InFirst, int32 InNumRays)
		: First(InFirst)
		, NumRays(InNumRays)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Lidar")
	int32 First {0};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Lidar")
	int32 NumRays {0};
};
