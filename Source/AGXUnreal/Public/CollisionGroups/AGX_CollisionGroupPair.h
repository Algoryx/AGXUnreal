// Copyright 2025, Algoryx Simulation AB.

#pragma once

#include "CoreMinimal.h"
#include "AGX_CollisionGroupPair.generated.h"

USTRUCT()
struct AGXUNREAL_API FAGX_CollisionGroupPair
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "AGX Collision Groups")
	FName Group1;

	UPROPERTY(EditAnywhere, Category = "AGX Collision Groups")
	FName Group2;

	const bool operator==(const FAGX_CollisionGroupPair& Other) const
	{
		return Group1.IsEqual(Other.Group1) && Group2.IsEqual(Other.Group2);
	}

	/**
	 * If any of the two group permutations match, returns true. False otherwise.
	 */
	bool IsEqual(const FName& GroupA, const FName& GroupB) const;

	bool IsIn(const TArray<FAGX_CollisionGroupPair>& Pairs) const;
};
