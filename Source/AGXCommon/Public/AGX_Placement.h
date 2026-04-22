// Copyright 2026, Algoryx Simulation AB.

#pragma once

// Unreal Engine includes.
#include "CoreMinimal.h"
#include "UObject/Class.h"

#include "AGX_Placement.generated.h"

/**
 * A location + rotation pair.
 * Use this instead of FTransform when scale should not be represented.
 */
USTRUCT(BlueprintType)
struct AGXCOMMON_API FAGX_Placement
{
	GENERATED_BODY()

	FAGX_Placement() = default;

	explicit FAGX_Placement(const FTransform& Transform)
		: Location(Transform.GetLocation())
		, Rotation(Transform.Rotator())
	{
	}

	FTransform ToTransform() const
	{
		return FTransform(Rotation, Location);
	}

	/** Location component. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX")
	FVector Location = FVector::ZeroVector;

	/** Rotation component. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AGX")
	FRotator Rotation = FRotator::ZeroRotator;
};
