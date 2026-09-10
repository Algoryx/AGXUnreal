// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"

#include "AGX_ColorMappingMatrix.generated.h"

USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_ColorMappingMatrix
{
	GENERATED_BODY()

	/**
	 * The first row of the color mapping matrix.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Camera")
	FLinearColor Row0 {1.0f, 0.0f, 0.0f, 0.0f};

	/**
	 * The second row of the color mapping matrix.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Camera")
	FLinearColor Row1 {0.0f, 1.0f, 0.0f, 0.0f};

	/**
	 * The third row of the color mapping matrix.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Camera")
	FLinearColor Row2 {0.0f, 0.0f, 1.0f, 0.0f};

	/**
	 * The fourth row of the color mapping matrix.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX Camera")
	FLinearColor Row3 {0.0f, 0.0f, 0.0f, 1.0f};
};
